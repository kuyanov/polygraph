#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>
#include <rapidjson/document.h>

template <class T>
void Deserialize(T &data, const rapidjson::Value &value);

template <>
inline void Deserialize<bool>(bool &data, const rapidjson::Value &value) {
    data = value.GetBool();
}

template <>
inline void Deserialize<int>(int &data, const rapidjson::Value &value) {
    data = value.GetInt();
}

template <>
inline void Deserialize<int64_t>(int64_t &data, const rapidjson::Value &value) {
    data = value.GetInt64();
}

template <>
inline void Deserialize<uint64_t>(uint64_t &data, const rapidjson::Value &value) {
    data = value.GetUint64();
}

template <>
inline void Deserialize<std::string>(std::string &data, const rapidjson::Value &value) {
    data = value.GetString();
}

template <class T>
void Deserialize(std::optional<T> &data, const rapidjson::Value &value) {
    if (!value.IsNull()) {
        data.template emplace();
        Deserialize(data.value(), value);
    } else {
        data.reset();
    }
}

template <class T>
void Deserialize(std::vector<T> &data, const rapidjson::Value &value) {
    data.resize(value.GetArray().Size());
    for (size_t i = 0; i < data.size(); ++i) {
        Deserialize(data[i], value.GetArray()[i]);
    }
}

template <class T>
rapidjson::Value Serialize(const T &data, rapidjson::Document::AllocatorType &alloc);

template <>
inline rapidjson::Value Serialize<bool>(const bool &data, rapidjson::Document::AllocatorType &) {
    rapidjson::Value value;
    value.SetBool(data);
    return value;
}

template <>
inline rapidjson::Value Serialize<int>(const int &data, rapidjson::Document::AllocatorType &) {
    rapidjson::Value value;
    value.SetInt(data);
    return value;
}

template <>
inline rapidjson::Value Serialize<int64_t>(const int64_t &data,
                                           rapidjson::Document::AllocatorType &) {
    rapidjson::Value value;
    value.SetInt64(data);
    return value;
}

template <>
inline rapidjson::Value Serialize<uint64_t>(const uint64_t &data,
                                            rapidjson::Document::AllocatorType &) {
    rapidjson::Value value;
    value.SetUint64(data);
    return value;
}

template <>
inline rapidjson::Value Serialize<std::string>(const std::string &data,
                                               rapidjson::Document::AllocatorType &alloc) {
    rapidjson::Value value;
    value.SetString(data.c_str(), alloc);
    return value;
}

template <class T>
rapidjson::Value Serialize(const std::optional<T> &data,
                           rapidjson::Document::AllocatorType &alloc) {
    if (data.has_value()) {
        return Serialize(data.value(), alloc);
    } else {
        return rapidjson::Value(rapidjson::kNullType);
    }
}

template <class T>
rapidjson::Value Serialize(const std::vector<T> &data, rapidjson::Document::AllocatorType &alloc) {
    rapidjson::Value value(rapidjson::kArrayType);
    for (const T &elem : data) {
        value.PushBack(Serialize(elem, alloc), alloc);
    }
    return value;
}

template <class T>
rapidjson::Document Serialize(const T &data) {
    rapidjson::Document document;
    document.CopyFrom(Serialize(data, document.GetAllocator()), document.GetAllocator());
    return document;
}
