#include "run_request.h"

void RunRequest::LoadFromValue(const rapidjson::Value &json) {
    container = json["container"].GetString();
    auto tasks_array = json["tasks"].GetArray();
    tasks.resize(tasks_array.Size());
    for (size_t i = 0; i < tasks.size(); ++i) {
        tasks[i].LoadFromValue(tasks_array[i]);
    }
}

rapidjson::Value RunRequest::DumpToValue(rapidjson::Document::AllocatorType &alloc) const {
    rapidjson::Value json(rapidjson::kObjectType);
    json.AddMember("container", rapidjson::Value().SetString(container.c_str(), alloc), alloc);
    rapidjson::Value tasks_array(rapidjson::kArrayType);
    for (const auto &task : tasks) {
        tasks_array.PushBack(task.DumpToValue(alloc), alloc);
    }
    json.AddMember("tasks", tasks_array, alloc);
    return json;
}
