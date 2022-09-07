#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>
#include <rapidjson/document.h>

template <class T>
void Load(T &data, const rapidjson::Value &value);

template <>
inline void Load<bool>(bool &data, const rapidjson::Value &value) {
    data = value.GetBool();
}

template <>
inline void Load<int>(int &data, const rapidjson::Value &value) {
    data = value.GetInt();
}

template <>
inline void Load<int64_t>(int64_t &data, const rapidjson::Value &value) {
    data = value.GetInt64();
}

template <>
inline void Load<std::string>(std::string &data, const rapidjson::Value &value) {
    data = value.GetString();
}

template <class T>
void Load(std::optional<T> &data, const rapidjson::Value &value) {
    if (!value.IsNull()) {
        data.template emplace();
        Load(data.value(), value);
    } else {
        data.reset();
    }
}

template <class T>
void Load(std::vector<T> &data, const rapidjson::Value &value) {
    data.resize(value.GetArray().Size());
    for (size_t i = 0; i < data.size(); ++i) {
        Load(data[i], value.GetArray()[i]);
    }
}

template <class T>
rapidjson::Value Dump(const T &data, rapidjson::Document::AllocatorType &alloc);

template <>
inline rapidjson::Value Dump<bool>(const bool &data, rapidjson::Document::AllocatorType &alloc) {
    rapidjson::Value value;
    value.SetBool(data);
    return value;
}

template <>
inline rapidjson::Value Dump<int>(const int &data, rapidjson::Document::AllocatorType &alloc) {
    rapidjson::Value value;
    value.SetInt(data);
    return value;
}

template <>
inline rapidjson::Value Dump<int64_t>(const int64_t &data,
                                      rapidjson::Document::AllocatorType &alloc) {
    rapidjson::Value value;
    value.SetInt64(data);
    return value;
}

template <>
inline rapidjson::Value Dump<std::string>(const std::string &data,
                                          rapidjson::Document::AllocatorType &alloc) {
    rapidjson::Value value;
    value.SetString(data.c_str(), alloc);
    return value;
}

template <class T>
rapidjson::Value Dump(const std::optional<T> &data, rapidjson::Document::AllocatorType &alloc) {
    if (data.has_value()) {
        return Dump(data.value(), alloc);
    } else {
        return rapidjson::Value(rapidjson::kNullType);
    }
}

template <class T>
rapidjson::Value Dump(const std::vector<T> &data, rapidjson::Document::AllocatorType &alloc) {
    rapidjson::Value value(rapidjson::kArrayType);
    for (const T &elem : data) {
        value.PushBack(Dump(elem, alloc), alloc);
    }
    return value;
}

template <class T>
rapidjson::Document Dump(const T &data) {
    rapidjson::Document document;
    document.CopyFrom(Dump(data, document.GetAllocator()), document.GetAllocator());
    return document;
}
