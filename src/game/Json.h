#pragma once

#include <string>
#include <unordered_map>
#include <vector>

struct JsonValue {
    enum class Type {
        kNull,
        kBool,
        kNumber,
        kString,
        kArray,
        kObject
    };

    Type type = Type::kNull;
    bool bool_value = false;
    double number_value = 0.0;
    std::string string_value;
    std::vector<JsonValue> array_value;
    std::unordered_map<std::string, JsonValue> object_value;

    const JsonValue* Get(const std::string& key) const;
    const JsonValue* GetIndex(size_t index) const;

    bool IsObject() const { return type == Type::kObject; }
    bool IsArray() const { return type == Type::kArray; }
    bool IsString() const { return type == Type::kString; }
    bool IsNumber() const { return type == Type::kNumber; }
    bool IsBool() const { return type == Type::kBool; }
};

bool ParseJson(const std::string& input, JsonValue* out_value, std::string* out_error);
