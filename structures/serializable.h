#pragma once

#include <rapidjson/document.h>

class Serializable {
public:
    void Load(const rapidjson::Document &document) {
        LoadFromValue(document);
    }

    rapidjson::Document Dump() const {
        rapidjson::Document document;
        document.CopyFrom(DumpToValue(document.GetAllocator()), document.GetAllocator());
        return document;
    }

    virtual void LoadFromValue(const rapidjson::Value &json) = 0;
    virtual rapidjson::Value DumpToValue(rapidjson::Document::AllocatorType &alloc) const = 0;
};
