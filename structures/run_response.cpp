#include "run_response.h"

void RunResponse::LoadFromValue(const rapidjson::Value &json) {
    has_error = json.HasMember("error");
    if (has_error) {
        error = json["error"].GetString();
        return;
    }
    auto statuses_array = json["statuses"].GetArray();
    statuses.resize(statuses_array.Size());
    for (size_t i = 0; i < statuses.size(); ++i) {
        statuses[i].LoadFromValue(statuses_array[i]);
    }
}

rapidjson::Value RunResponse::DumpToValue(rapidjson::Document::AllocatorType &alloc) const {
    rapidjson::Value json(rapidjson::kObjectType);
    if (has_error) {
        json.AddMember("error", rapidjson::Value().SetString(error.c_str(), alloc), alloc);
        return json;
    }
    rapidjson::Value statuses_array(rapidjson::kArrayType);
    for (const auto &status : statuses) {
        statuses_array.PushBack(status.DumpToValue(alloc), alloc);
    }
    json.AddMember("statuses", statuses_array, alloc);
    return json;
}

BlockRunResponse::BlockRunResponse(const RunResponse &other) : RunResponse(other) {
}

void BlockRunResponse::LoadFromValue(const rapidjson::Value &json) {
    RunResponse::LoadFromValue(json);
    block_id = json["block-id"].GetInt();
}

rapidjson::Value BlockRunResponse::DumpToValue(rapidjson::Document::AllocatorType &alloc) const {
    auto json = RunResponse::DumpToValue(alloc);
    json.AddMember("block-id", rapidjson::Value().SetInt(block_id), alloc);
    return json;
}
