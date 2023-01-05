#include <string>
#include <string_view>
#include <App.h>

#include "config.h"
#include "constants.h"
#include "error.h"
#include "logger.h"
#include "run.h"
#include "scheduler.h"

void Run() {
    SchemaValidator workflow_validator(paths::kDataDir + "/schema/workflow.json");
    Scheduler scheduler;

    uWS::App().post("/submit", [&](auto *res, auto *req) {
        std::string workflow_text;
        res->onAborted([] {});
        res->onData([&, res, workflow_text = std::move(workflow_text)]
                    (std::string_view chunk, bool is_last) mutable {
            if (workflow_text.size() + chunk.size() > Config::Get().max_payload_length) {
                res->writeStatus(http_response::kRequestEntityTooLarge)->end("", true);
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
                res->writeStatus(http_response::kBadRequest)->end(
                    errors::kParseErrorPrefix + error.message);
            } catch (const ValidationError &error) {
                res->writeStatus(http_response::kBadRequest)->end(
                    errors::kValidationErrorPrefix + error.message);
            }
        });
    }).ws<RunnerPerSocketData>("/runner/:partition", {
        .maxPayloadLength = Config::Get().max_payload_length,
        .upgrade = [&](auto *res, auto *req, auto *context) {
            std::string partition(req->getParameter(0));
            res->template upgrade<RunnerPerSocketData>({ .partition = partition },
                req->getHeader("sec-websocket-key"),
                req->getHeader("sec-websocket-protocol"),
                req->getHeader("sec-websocket-extensions"),
                context
            );
        },
        .open = [&](auto *ws) {
            scheduler.JoinRunner(ws);
        },
        .message = [&](auto *ws, std::string_view message, uWS::OpCode op_code) {
            WorkflowState *workflow_ptr = ws->getUserData()->workflow_ptr;
            workflow_ptr->OnStatus(ws, message);
        },
        .close = [&](auto *ws, int code, std::string_view message) {
            scheduler.LeaveRunner(ws);
        }
    }).ws<ClientPerSocketData>("/workflow/:id", {
        .maxPayloadLength = Config::Get().max_payload_length,
        .idleTimeout = Config::Get().idle_timeout,
        .upgrade = [&](auto *res, auto *req, auto *context) {
            std::string workflow_id(req->getParameter(0));
            WorkflowState *workflow_ptr = scheduler.FindWorkflow(workflow_id);
            if (!workflow_ptr) {
                res->writeStatus(http_response::kNotFound)->end();
                return;
            }
            res->template upgrade<ClientPerSocketData>({ .workflow_ptr = workflow_ptr },
                req->getHeader("sec-websocket-key"),
                req->getHeader("sec-websocket-protocol"),
                req->getHeader("sec-websocket-extensions"),
                context
            );
        },
        .open = [&](auto *ws) {
            scheduler.JoinClient(ws);
        },
        .message = [&](auto *ws, std::string_view message, uWS::OpCode op_code) {
            WorkflowState *workflow_ptr = ws->getUserData()->workflow_ptr;
            try {
                if (message == signals::kRun) {
                    workflow_ptr->Run();
                } else if (message == signals::kStop) {
                    workflow_ptr->Stop();
                } else {
                    throw APIError(errors::kUndefinedCommand);
                }
            } catch (const APIError &error) {
                ws->send(errors::kAPIErrorPrefix + error.message);
            }
        },
        .close = [&](auto *ws, int code, std::string_view message) {
            scheduler.LeaveClient(ws);
        }
    }).listen(Config::Get().host, Config::Get().port, [&](auto *listen_socket) {
        if (listen_socket) {
            Log("Listening on ", Config::Get().host, ":", Config::Get().port);
        } else {
            Log("Failed to listen on ", Config::Get().host, ":", Config::Get().port);
            exit(1);
        }
    }).run();
}
