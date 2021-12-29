#include <iostream>
#include <utility>

#include <App.h>

#include "config.h"
#include "graph.h"
#include "run.h"
#include "schema_validator.h"
#include "socket_group.h"
#include "uuid.h"

const char *http_bad_request = "400 Bad Request";
const char *http_not_found = "404 Not Found";
const char *http_request_entity_too_large = "413 Request Entity Too Large";

void Run(const Config &config) {
    SchemaValidator graph_validator(config.graph_schema_file.c_str());
    GraphStorage graph_storage;
    SocketGroup users, runners;

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
                auto graph = graph_validator.ParseAndValidate(graph_json);
                std::string graph_id = GenerateUuid();
                graph_storage.InitGraph(graph_id, graph);
                res->end(graph_id);
            } catch (const ParseError &error) {
                res->writeStatus(http_bad_request)->end("Could not parse json: " + error.message);
            } catch (const ValidationError &error) {
                res->writeStatus(http_bad_request)->end("Invalid document: " + error.message);
            }
        });
    }).ws<UserData>("/graph/:id", {
        .maxPayloadLength = config.max_payload_size,
        .upgrade = [&](auto *res, auto *req, auto *context) {
            std::string graph_id(req->getParameter(0));
            if (!graph_storage.Contains(graph_id)) {
                res->writeStatus(http_not_found)->end();
                return;
            }
            res->template upgrade<UserData>({ .group = graph_id },
                req->getHeader("sec-websocket-key"),
                req->getHeader("sec-websocket-protocol"),
                req->getHeader("sec-websocket-extensions"),
                context
            );
        },
        .open = [&](auto *ws) {
            users.Join(ws);
        },
        .message = [&](auto *ws, std::string_view message, uWS::OpCode op_code) {
            if (message == "run") {
                // TODO
            } else if (message == "stop") {
                // TODO
            }
        },
        .close = [&](auto *ws, int code, std::string_view message) {
            users.Leave(ws);
        }
    }).listen(config.host, config.port, [&](auto *listen_socket) {
        if (listen_socket) {
            std::cout << "Listening on " << config.host << ":" << config.port << std::endl;
        } else {
            std::cerr << "Failed to listen on " << config.host << ":" << config.port << std::endl;
        }
    }).run();
}
