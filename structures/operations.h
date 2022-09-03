#pragma once

#include <rapidjson/document.h>

template <class T>
void Load(T &object, const rapidjson::Value &json);

template <class T>
rapidjson::Value Dump(const T &object, rapidjson::Document::AllocatorType &alloc);

template <class T>
rapidjson::Document Dump(const T &object) {
    rapidjson::Document document;
    document.CopyFrom(Dump(object, document.GetAllocator()), document.GetAllocator());
    return document;
}
