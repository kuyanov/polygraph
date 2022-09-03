#include "operations.h"
#include "run_response.h"

template <>
void Load<RunResponse>(RunResponse &object, const rapidjson::Value &json) {
    object.has_error = json.HasMember("error");
    if (object.has_error) {
        object.error = json["error"].GetString();
        return;
    }
    auto results_array = json["results"].GetArray();
    object.results.resize(results_array.Size());
    for (size_t i = 0; i < object.results.size(); ++i) {
        Load(object.results[i], results_array[i]);
    }
}

template <>
rapidjson::Value Dump<RunResponse>(const RunResponse &object,
                                   rapidjson::Document::AllocatorType &alloc) {
    rapidjson::Value json(rapidjson::kObjectType);
    if (object.has_error) {
        json.AddMember("error", rapidjson::Value().SetString(object.error.c_str(), alloc), alloc);
        return json;
    }
    rapidjson::Value results_array(rapidjson::kArrayType);
    for (const auto &result : object.results) {
        results_array.PushBack(Dump(result, alloc), alloc);
    }
    json.AddMember("results", results_array, alloc);
    return json;
}

template <>
void Load<BlockRunResponse>(BlockRunResponse &object, const rapidjson::Value &json) {
    Load(object.run_response, json);
    object.block_id = json["block-id"].GetInt();
}

template <>
rapidjson::Value Dump<BlockRunResponse>(const BlockRunResponse &object,
                                        rapidjson::Document::AllocatorType &alloc) {
    auto json = Dump(object.run_response, alloc);
    json.AddMember("block-id", rapidjson::Value().SetInt(object.block_id), alloc);
    return json;
}
