#include <string>
#include <string_view>
#include <uWebSockets/App.h>

#include "config.h"
#include "constants.h"
#include "error.h"
#include "logger.h"
#include "run.h"
#include "scheduler.h"

const std::string kHost = Config::Get().host;
const int kPort = Config::Get().port;

static SchemaValidator graph_validator("graph.json");
static Scheduler scheduler;

void Run() {
    Logger::Get().SetName("scheduler");
    uWS::App().post("/submit", [&](auto *res, auto *req) {
        std::string graph_text;
        res->onAborted([] {});
        res->onData([&, res, graph_text = std::move(graph_text)]
                    (std::string_view chunk, bool is_last) mutable {
            if (graph_text.size() + chunk.size() > Config::Get().max_payload_size) {
                res->writeStatus(http_response::kRequestEntityTooLarge)->end("", true);
                return;
            }
            graph_text.append(chunk);
            if (!is_last) {
                return;
            }
            try {
                auto document = graph_validator.ParseAndValidate(graph_text);
                std::string graph_id = scheduler.AddGraph(document);
                res->end(graph_id);
            } catch (const ParseError &error) {
                res->writeStatus(http_response::kBadRequest)->end(
                    errors::kParseErrorPrefix + error.message);
            } catch (const ValidationError &error) {
                res->writeStatus(http_response::kBadRequest)->end(
                    errors::kValidationErrorPrefix + error.message);
            }
        });
    }).ws<RunnerPerSocketData>("/runner/:partition", {
        .maxPayloadLength = Config::Get().max_payload_size,
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
            GraphState *graph_ptr = ws->getUserData()->graph_ptr;
            graph_ptr->OnResult(ws, message);
        },
        .close = [&](auto *ws, int code, std::string_view message) {
            scheduler.LeaveRunner(ws);
        }
    }).ws<ClientPerSocketData>("/graph/:id", {
        .maxPayloadLength = Config::Get().max_payload_size,
        .upgrade = [&](auto *res, auto *req, auto *context) {
            std::string graph_id(req->getParameter(0));
            GraphState *graph_ptr = scheduler.FindGraph(graph_id);
            if (!graph_ptr) {
                res->writeStatus(http_response::kNotFound)->end();
                return;
            }
            res->template upgrade<ClientPerSocketData>({ .graph_ptr = graph_ptr },
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
            GraphState *graph_ptr = ws->getUserData()->graph_ptr;
            try {
                if (message == signals::kRun) {
                    graph_ptr->Run();
                } else if (message == signals::kStop) {
                    graph_ptr->Stop();
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
    }).listen(kHost, kPort, [&](auto *listen_socket) {
        if (listen_socket) {
            Log("Listening on ", kHost, ":", kPort);
        } else {
            Log("Failed to listen on ", kHost, ":", kPort);
        }
    }).run();
}
