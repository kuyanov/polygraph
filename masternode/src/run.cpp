#include <iostream>

#include <App.h>

#include "config.h"
#include "constants.h"
#include "error.h"
#include "graph.h"
#include "run.h"
#include "scheduler.h"
#include "schema_validator.h"

void Run() {
    static SchemaValidator graph_validator("graph.json");
    static Scheduler scheduler;

    uWS::App().post("/submit", [&](auto *res, auto *req) {
        std::string graph_json;
        res->onAborted([] {});
        res->onData([&, res, graph_json = std::move(graph_json)]
                    (std::string_view chunk, bool is_last) mutable {
            if (graph_json.size() + chunk.size() > Config::Instance().max_payload_size) {
                res->writeStatus(http_response::kRequestEntityTooLarge)->end("", true);
                return;
            }
            graph_json.append(chunk);
            if (!is_last) {
                return;
            }
            try {
                auto graph_document = graph_validator.ParseAndValidate(graph_json);
                auto graph = Graph(graph_document);
                std::string graph_id = scheduler.AddGraph(std::move(graph));
                res->end(graph_id);
            } catch (const ParseError &error) {
                res->writeStatus(http_response::kBadRequest)->end(
                    errors::kParseErrorPrefix + error.message);
            } catch (const ValidationError &error) {
                res->writeStatus(http_response::kBadRequest)->end(
                    errors::kValidationErrorPrefix + error.message);
            } catch (const SemanticError &error) {
                res->writeStatus(http_response::kBadRequest)->end(
                    errors::kSemanticErrorPrefix + error.message);
            }
        });
    }).ws<RunnerPerSocketData>("/runner/:group", {
        .maxPayloadLength = Config::Instance().max_payload_size,
        .upgrade = [&](auto *res, auto *req, auto *context) {
            std::string runner_group(req->getParameter(0));
            res->template upgrade<RunnerPerSocketData>({ .runner_group = runner_group },
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
            graph_ptr->CompleteBlock(ws, message);
        },
        .close = [&](auto *ws, int code, std::string_view message) {
            scheduler.LeaveRunner(ws);
        }
    }).ws<ClientPerSocketData>("/graph/:id", {
        .maxPayloadLength = Config::Instance().max_payload_size,
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
            if (message == signals::kGraphRun) {
                if (!graph_ptr->IsRunning()) {
                    graph_ptr->Run();
                }
            } else if (message == signals::kGraphStop) {
                if (graph_ptr->IsRunning()) {
                    graph_ptr->Stop();
                }
            }
        },
        .close = [&](auto *ws, int code, std::string_view message) {
            scheduler.LeaveClient(ws);
        }
    }).listen(Config::Instance().host, Config::Instance().port, [&](auto *listen_socket) {
        if (listen_socket) {
            std::cout << "Listening on " << Config::Instance().host << ":"
                      << Config::Instance().port << std::endl;
        } else {
            std::cerr << "Failed to listen on " << Config::Instance().host << ":"
                      << Config::Instance().port << std::endl;
        }
    }).run();
}
