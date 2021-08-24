#include <iostream>
#include "App.h"
#include "config.h"
#include "schema_validator.h"
#include "uuid.h"

using namespace std;

const char *CONFIG_FILE = "config.json";
const char *GRAPH_SCHEMA_FILE = "schema/graph.json";

struct UserData {
};

int main() {
    Config config(CONFIG_FILE);
    SchemaValidator graphValidator(GRAPH_SCHEMA_FILE);

    uWS::App().get("/", [](auto *res, auto *req) {
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
            if (isLast) {
                rapidjson::Document graph;
                if (graph.Parse(graphJson.c_str()).HasParseError()) {
                    res->writeStatus("400 Bad Request")->end("Not a json");
                    return;
                }
                if (!graphValidator.validate(graph)) {
                    res->writeStatus("400 Bad Request")->end("Invalid document: " + graphValidator.validationError);
                    return;
                }
                string graphId = uuid::generate();
                res->end(graphId);
            }
        });
    }).listen(config.host, config.port, [&](auto *listen_socket) {
        if (listen_socket) {
            cout << "Listening on " << config.host << ":" << config.port << endl;
        } else {
            cout << "Failed to listen on " << config.host << ":" << config.port << endl;
        }
    }).run();
}
