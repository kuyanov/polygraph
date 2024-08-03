#include <cstdlib>
#include <string>
#include <string_view>
#include <App.h>

#include "config.h"
#include "definitions.h"
#include "error.h"
#include "json.h"
#include "logger.h"
#include "run.h"
#include "scheduler.h"

const std::string kListenHost = "0.0.0.0";

void Run() {
    SchemaValidator workflow_validator(GetDataDir() + "/schema/workflow.json");
    Scheduler scheduler;

    uWS::App()
        .post(
            "/submit",
            [&](auto *res, auto *req) {
                std::string workflow_text;
                res->onAborted([] {});
                res->onData([&, res, workflow_text = std::move(workflow_text)](
                                std::string_view chunk, bool is_last) mutable {
                    if (workflow_text.size() + chunk.size() >
                        SchedulerConfig::Get().max_payload_length) {
                        res->writeStatus(HTTP_REQUEST_ENTITY_TOO_LARGE)->end("", true);
                        return;
                    }
                    workflow_text.append(chunk);
                    if (!is_last) {
                        return;
                    }
                    try {
                        auto document = workflow_validator.ParseAndValidate(workflow_text);
                        std::string workflow_id = scheduler.AddWorkflow(document);
                        res->end(workflow_id);
                    } catch (const ParseError &error) {
                        res->writeStatus(HTTP_BAD_REQUEST)->end(PARSE_ERROR_PREFIX + error.message);
                    } catch (const ValidationError &error) {
                        res->writeStatus(HTTP_BAD_REQUEST)
                            ->end(VALIDATION_ERROR_PREFIX + error.message);
                    }
                });
            })
        .ws<RunnerPerSocketData>(
            "/runner/:partition",
            {.maxPayloadLength = SchedulerConfig::Get().max_payload_length,
             .upgrade =
                 [&](auto *res, auto *req, auto *context) {
                     std::string partition(req->getParameter(0));
                     res->template upgrade<RunnerPerSocketData>(
                         {.partition = partition}, req->getHeader("sec-websocket-key"),
                         req->getHeader("sec-websocket-protocol"),
                         req->getHeader("sec-websocket-extensions"), context);
                 },
             .open = [&](auto *ws) { scheduler.JoinRunner(ws); },
             .message =
                 [&](auto *ws, std::string_view message, uWS::OpCode op_code) {
                     WorkflowState *workflow_ptr = ws->getUserData()->workflow_ptr;
                     workflow_ptr->OnStatus(ws, message);
                 },
             .close = [&](auto *ws, int code,
                          std::string_view message) { scheduler.LeaveRunner(ws); }})
        .ws<ClientPerSocketData>(
            "/workflow/:id",
            {.maxPayloadLength = SchedulerConfig::Get().max_payload_length,
             .idleTimeout = SchedulerConfig::Get().idle_timeout,
             .upgrade =
                 [&](auto *res, auto *req, auto *context) {
                     std::string workflow_id(req->getParameter(0));
                     WorkflowState *workflow_ptr = scheduler.FindWorkflow(workflow_id);
                     if (!workflow_ptr) {
                         res->writeStatus(HTTP_NOT_FOUND)->end();
                         return;
                     }
                     res->template upgrade<ClientPerSocketData>(
                         {.workflow_ptr = workflow_ptr}, req->getHeader("sec-websocket-key"),
                         req->getHeader("sec-websocket-protocol"),
                         req->getHeader("sec-websocket-extensions"), context);
                 },
             .open = [&](auto *ws) { scheduler.JoinClient(ws); },
             .message =
                 [&](auto *ws, std::string_view message, uWS::OpCode op_code) {
                     WorkflowState *workflow_ptr = ws->getUserData()->workflow_ptr;
                     try {
                         if (message == RUN_SIGNAL) {
                             workflow_ptr->Run();
                         } else if (message == STOP_SIGNAL) {
                             workflow_ptr->Stop();
                         } else {
                             throw APIError(UNDEFINED_COMMAND_ERROR);
                         }
                     } catch (const APIError &error) {
                         ws->send(API_ERROR_PREFIX + error.message, uWS::OpCode::TEXT);
                     }
                 },
             .close = [&](auto *ws, int code,
                          std::string_view message) { scheduler.LeaveClient(ws); }})
        .listen(kListenHost, SchedulerConfig::Get().port,
                [&](auto *listen_socket) {
                    if (listen_socket) {
                        Log("Listening on ", kListenHost, ":", SchedulerConfig::Get().port);
                    } else {
                        Log("Failed to listen on ", kListenHost, ":", SchedulerConfig::Get().port);
                        exit(EXIT_FAILURE);
                    }
                })
        .run();
}
