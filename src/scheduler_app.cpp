#include <csignal>
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
#include "scheduler_app.h"
#include "submit_response.h"

SchedulerApp::SchedulerApp() : workflow_validator_(SCHEMA_DIR "/workflow.json") {
}

void SchedulerInterruptHandler(int signum) {
    Log("Terminated with signal ", signum);
    exit(signum);
}

void SchedulerApp::Run() {
    Logger::Get().SetName("scheduler");
    signal(SIGINT, SchedulerInterruptHandler);
    signal(SIGTERM, SchedulerInterruptHandler);
    uWS::App()
        .post("/submit",
              [&](auto *res, auto *req) {
                  std::string workflow_text;
                  res->onAborted([] {});
                  res->onData([&, res, workflow_text = std::move(workflow_text)](
                                  std::string_view chunk, bool is_last) mutable {
                      if (workflow_text.size() + chunk.size() >
                          Config::Get().scheduler_max_payload_length) {
                          res->writeStatus(HTTP_REQUEST_ENTITY_TOO_LARGE)->end("", true);
                          return;
                      }
                      workflow_text.append(chunk);
                      if (!is_last) {
                          return;
                      }
                      SubmitResponse submit_response;
                      try {
                          auto document = workflow_validator_.ParseAndValidate(workflow_text);
                          std::string workflow_id = scheduler_.AddWorkflow(document);
                          submit_response.status = SUBMIT_ACCEPTED;
                          submit_response.data = workflow_id;
                      } catch (const ParseError &error) {
                          res->writeStatus(HTTP_BAD_REQUEST);
                          submit_response.status = SUBMIT_PARSE_ERROR;
                          submit_response.data = error.message;
                      } catch (const ValidationError &error) {
                          res->writeStatus(HTTP_BAD_REQUEST);
                          submit_response.status = SUBMIT_VALIDATION_ERROR;
                          submit_response.data = error.message;
                      }
                      res->end(StringifyJSON(Serialize(submit_response)));
                      Log("Received workflow, status = '", submit_response.status, "', data = '",
                          submit_response.data, "'");
                  });
              })
        .ws<RunnerPerSocketData>(
            "/runner/:partition/:id",
            {.maxPayloadLength =
                 static_cast<unsigned int>(Config::Get().scheduler_max_payload_length),
             .upgrade =
                 [&](auto *res, auto *req, auto *context) {
                     std::string partition(req->getParameter("partition"));
                     int runner_id = std::stoi(std::string(req->getParameter("id")));
                     res->template upgrade<RunnerPerSocketData>(
                         {.partition = partition, .runner_id = runner_id},
                         req->getHeader("sec-websocket-key"),
                         req->getHeader("sec-websocket-protocol"),
                         req->getHeader("sec-websocket-extensions"), context);
                 },
             .open =
                 [&](auto *ws) {
                     Log("Runner ", ws->getUserData()->runner_id, " connected, partition = '",
                         ws->getUserData()->partition, "'");
                     scheduler_.JoinRunner(ws);
                 },
             .message =
                 [&](auto *ws, std::string_view message, uWS::OpCode op_code) {
                     Log("Runner ", ws->getUserData()->runner_id, " finished");
                     WorkflowState *workflow_ptr = ws->getUserData()->workflow_ptr;
                     workflow_ptr->OnStatus(ws, message);
                 },
             .close =
                 [&](auto *ws, int code, std::string_view message) {
                     Log("Runner ", ws->getUserData()->runner_id, " disconnected");
                     scheduler_.LeaveRunner(ws);
                 }})
        .ws<ClientPerSocketData>(
            "/workflow/:id",
            {.maxPayloadLength =
                 static_cast<unsigned int>(Config::Get().scheduler_max_payload_length),
             .idleTimeout = static_cast<unsigned short>(Config::Get().scheduler_idle_timeout_s),
             .upgrade =
                 [&](auto *res, auto *req, auto *context) {
                     std::string workflow_id(req->getParameter("id"));
                     WorkflowState *workflow_ptr = scheduler_.FindWorkflow(workflow_id);
                     if (!workflow_ptr) {
                         res->writeStatus(HTTP_NOT_FOUND)->end();
                         return;
                     }
                     res->template upgrade<ClientPerSocketData>(
                         {.workflow_ptr = workflow_ptr}, req->getHeader("sec-websocket-key"),
                         req->getHeader("sec-websocket-protocol"),
                         req->getHeader("sec-websocket-extensions"), context);
                 },
             .open =
                 [&](auto *ws) {
                     Log("Workflow ", ws->getUserData()->workflow_ptr->workflow_id,
                         ": client connected");
                     scheduler_.JoinClient(ws);
                 },
             .message =
                 [&](auto *ws, std::string_view message, uWS::OpCode op_code) {
                     WorkflowState *workflow_ptr = ws->getUserData()->workflow_ptr;
                     Log("Workflow ", workflow_ptr->workflow_id, ": received signal '", message,
                         "'");
                     try {
                         if (message == RUN_SIGNAL) {
                             workflow_ptr->Run();
                         } else if (message == STOP_SIGNAL) {
                             workflow_ptr->Stop();
                         } else {
                             throw RuntimeError(UNDEFINED_COMMAND_ERROR);
                         }
                     } catch (const RuntimeError &error) {
                         ws->send(ERROR_SIGNAL + std::string(" ") + error.message,
                                  uWS::OpCode::TEXT);
                         Log("Workflow ", workflow_ptr->workflow_id, ": runtime error '",
                             error.message, "'");
                     }
                 },
             .close =
                 [&](auto *ws, int code, std::string_view message) {
                     Log("Workflow ", ws->getUserData()->workflow_ptr->workflow_id,
                         ": client disconnected");
                     scheduler_.LeaveClient(ws);
                 }})
        .listen(listen_host_, Config::Get().port,
                [&](auto *listen_socket) {
                    if (listen_socket) {
                        Log("Listening on ", listen_host_, ":", Config::Get().port);
                    } else {
                        Log("Failed to listen on ", listen_host_, ":", Config::Get().port);
                        exit(EXIT_FAILURE);
                    }
                })
        .run();
}
