#include <iostream>
#include <utility>
#include "App.h"
#include "clients_group.h"
#include "config.h"
#include "graph.h"
#include "schema_validator.h"
#include "uuid.h"

const char *config_file = "config.json";
const char *graph_schema_file = "schema/graph.json";

int main() {
    Config config(config_file);
    SchemaValidator graph_validator(graph_schema_file);
    GraphsStorage graphs_storage;
    ClientsGroup users;

    uWS::SSLApp({
        .key_file_name = config.ssl_key_file_name.c_str(),
        .cert_file_name = config.ssl_cert_file_name.c_str()
    }).get("/", [](auto *res, auto *req) {
        res->end();
    }).post("/submit", [&](auto *res, auto *req) {
        std::string graph_json;
        res->onAborted([]() {});
        res->onData([&, res, graph_json = move(graph_json)](std::string_view chunk, bool is_last) mutable {
            if (graph_json.size() + chunk.size() > config.max_payload_size) {
                res->writeStatus("413 Request Entity Too Large")->end("", true);
                return;
            }
            graph_json.append(chunk);
            if (!is_last) {
                return;
            }
            try {
                auto graph = graph_validator.Parse(graph_json);
                std::string graph_id = uuid::Generate();
                graphs_storage.InitGraph(graph_id, graph);
                res->end(graph_id);
            } catch (const ParseError &error) {
                res->writeStatus("400 Bad Request")->end("Could not parse json: " + error.message);
            } catch (const ValidationError &error) {
                res->writeStatus("400 Bad Request")->end("Invalid document: " + error.message);
            }
        });
    }).ws<UserData>("/graph/:id", {
        .maxPayloadLength = config.max_payload_size,
        .upgrade = [&](auto *res, auto *req, auto *context) {
            std::string graph_id(req->getParameter(0));
            if (!graphs_storage.Contains(graph_id)) {
                res->writeStatus("404 Not Found")->end();
                return;
            }
            res->template upgrade<UserData>({
                .group = graph_id
            },
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
