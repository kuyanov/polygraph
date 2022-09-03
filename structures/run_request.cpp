#include "operations.h"
#include "run_request.h"

template <>
void Load<RunRequest>(RunRequest &object, const rapidjson::Value &json) {
    object.container = json["container"].GetString();
    auto tasks_array = json["tasks"].GetArray();
    object.tasks.resize(tasks_array.Size());
    for (size_t i = 0; i < object.tasks.size(); ++i) {
        Load(object.tasks[i], tasks_array[i]);
    }
}

template <>
rapidjson::Value Dump<RunRequest>(const RunRequest &object,
                                  rapidjson::Document::AllocatorType &alloc) {
    rapidjson::Value json(rapidjson::kObjectType);
    json.AddMember("container", rapidjson::Value().SetString(object.container.c_str(), alloc),
                   alloc);
    rapidjson::Value tasks_array(rapidjson::kArrayType);
    for (const auto &task : object.tasks) {
        tasks_array.PushBack(Dump(task, alloc), alloc);
    }
    json.AddMember("tasks", tasks_array, alloc);
    return json;
}
