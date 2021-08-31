#include <iostream>
#include <utility>
#include "App.h"
#include "clients_group.h"
#include "config.h"
#include "graph.h"
#include "schema_validator.h"
#include "uuid.h"

using namespace std;

const char *CONFIG_FILE = "config.json";
const char *GRAPH_SCHEMA_FILE = "schema/graph.json";

int main() {
    Config config(CONFIG_FILE);
    SchemaValidator graphValidator(GRAPH_SCHEMA_FILE);
    GraphsStorage graphsStorage;
    ClientsGroup users;

    uWS::SSLApp({
        .key_file_name = config.sslKeyFileName.c_str(),
        .cert_file_name = config.sslCertFileName.c_str()
    }).get("/", [](auto *res, auto *req) {
        res->end();
    }).post("/submit", [&](auto *res, auto *req) {
        string graphJson;
        res->onAborted([]() {});
        res->onData([&, res, graphJson = move(graphJson)](string_view chunk, bool isLast) mutable {
            if (graphJson.size() + chunk.size() > config.maxPayloadSize) {
                res->writeStatus("413 Request Entity Too Large")->end("", true);
                return;
            }
            graphJson.append(chunk);
            if (!isLast) {
                return;
            }
            try {
                auto graph = graphValidator.parse(graphJson);
                string graphId = uuid::generate();
                graphsStorage.initGraph(graphId, graph);
                res->end(graphId);
            } catch (const ParseError &error) {
                res->writeStatus("400 Bad Request")->end("Could not parse json: " + error.message);
            } catch (const ValidationError &error) {
                res->writeStatus("400 Bad Request")->end("Invalid document: " + error.message);
            }
        });
    }).ws<UserData>("/graph/:id", {
        .maxPayloadLength = config.maxPayloadSize,
        .upgrade = [&](auto *res, auto *req, auto *context) {
            string graphId(req->getParameter(0));
            if (!graphsStorage.contains(graphId)) {
                res->writeStatus("404 Not Found")->end();
                return;
            }
            res->template upgrade<UserData>({
                .group = graphId
            },
                req->getHeader("sec-websocket-key"),
                req->getHeader("sec-websocket-protocol"),
                req->getHeader("sec-websocket-extensions"),
                context
            );
        },
        .open = [&](auto *ws) {
            users.join(ws);
        },
        .message = [&](auto *ws, string_view message, uWS::OpCode opCode) {
            if (message == "run") {
                // TODO
            } else if (message == "stop") {
                // TODO
            }
        },
        .close = [&](auto *ws, int code, string_view message) {
            users.leave(ws);
        }
    }).listen(config.host, config.port, [&](auto *listen_socket) {
        if (listen_socket) {
            cout << "Listening on " << config.host << ":" << config.port << endl;
        } else {
            cerr << "Failed to listen on " << config.host << ":" << config.port << endl;
        }
    }).run();
}
