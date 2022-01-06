#include <iostream>
#include <utility>

#include <App.h>

#include "config.h"
#include "error.h"
#include "graph.h"
#include "run.h"
#include "scheduler.h"
#include "schema_validator.h"

const char *http_bad_request = "400 Bad Request";
const char *http_not_found = "404 Not Found";
const char *http_request_entity_too_large = "413 Request Entity Too Large";

void Run(const Config &config) {
    static SchemaValidator graph_validator(config.graph_schema_file.c_str());
    static Scheduler scheduler;

    uWS::App().post("/submit", [&](auto *res, auto *req) {
        std::string graph_json;
        res->onAborted([] {});
        res->onData([&, res, graph_json = std::move(graph_json)]
                    (std::string_view chunk, bool is_last) mutable {
            if (graph_json.size() + chunk.size() > config.max_payload_size) {
                res->writeStatus(http_request_entity_too_large)->end("", true);
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
                res->writeStatus(http_bad_request)->end("Could not parse json: " + error.message);
            } catch (const ValidationError &error) {
                res->writeStatus(http_bad_request)->end("Invalid document: " + error.message);
            } catch (const SemanticError &error) {
                res->writeStatus(http_bad_request)->end("Semantic error: " + error.message);
            }
        });
    }).ws<RunnerPerSocketData>("/runner/:group", {
        .maxPayloadLength = config.max_payload_size,
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
            scheduler.RunnerCompleted(ws, message);
        },
        .close = [&](auto *ws, int code, std::string_view message) {
            scheduler.LeaveRunner(ws);
        }
    }).ws<UserPerSocketData>("/graph/:id", {
        .maxPayloadLength = config.max_payload_size,
        .upgrade = [&](auto *res, auto *req, auto *context) {
            std::string graph_id(req->getParameter(0));
            if (!scheduler.GraphExists(graph_id)) {
                res->writeStatus(http_not_found)->end();
                return;
            }
            res->template upgrade<UserPerSocketData>({ .graph_id = graph_id },
                req->getHeader("sec-websocket-key"),
                req->getHeader("sec-websocket-protocol"),
                req->getHeader("sec-websocket-extensions"),
                context
            );
        },
        .open = [&](auto *ws) {
            scheduler.JoinUser(ws);
        },
        .message = [&](auto *ws, std::string_view message, uWS::OpCode op_code) {
            std::string graph_id = ws->getUserData()->graph_id;
            if (message == "run") {
                if (!scheduler.GraphRunning(graph_id)) {
                    scheduler.RunGraph(graph_id);
                }
            } else if (message == "stop") {
                if (scheduler.GraphRunning(graph_id)) {
                    scheduler.StopGraph(graph_id);
                }
            }
        },
        .close = [&](auto *ws, int code, std::string_view message) {
            scheduler.LeaveUser(ws);
        }
    }).listen(config.host, config.port, [&](auto *listen_socket) {
        if (!listen_socket) {
            std::cerr << "Failed to listen on " << config.host << ":" << config.port << std::endl;
        }
    }).run();
}
