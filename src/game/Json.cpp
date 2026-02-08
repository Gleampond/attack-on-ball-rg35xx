#include "Json.h"

#include <cctype>
#include <cstdlib>
#include <sstream>

namespace {
struct Parser {
    const std::string& input;
    size_t pos = 0;
    std::string error;

    explicit Parser(const std::string& text) : input(text) {}

    void SkipWhitespace() {
        while (pos < input.size() && std::isspace(static_cast<unsigned char>(input[pos]))) {
            pos++;
        }
    }

    bool Match(char expected) {
        SkipWhitespace();
        if (pos >= input.size() || input[pos] != expected) {
            return false;
        }
        pos++;
        return true;
    }

    bool ParseValue(JsonValue* out) {
        SkipWhitespace();
        if (pos >= input.size()) {
            error = "unexpected end of input";
            return false;
        }
        const char c = input[pos];
        if (c == '{') {
            return ParseObject(out);
        }
        if (c == '[') {
            return ParseArray(out);
        }
        if (c == '"') {
            std::string value;
            if (!ParseString(&value)) {
                return false;
            }
            out->type = JsonValue::Type::kString;
            out->string_value = std::move(value);
            return true;
        }
        if (c == '-' || std::isdigit(static_cast<unsigned char>(c))) {
            return ParseNumber(out);
        }
        if (c == 't' || c == 'f' || c == 'n') {
            return ParseLiteral(out);
        }
        std::ostringstream oss;
        oss << "unexpected character '" << c << "'";
        error = oss.str();
        return false;
    }

    bool ParseObject(JsonValue* out) {
        if (!Match('{')) {
            error = "expected '{'";
            return false;
        }
        out->type = JsonValue::Type::kObject;
        out->object_value.clear();
        SkipWhitespace();
        if (Match('}')) {
            return true;
        }
        while (true) {
            std::string key;
            if (!ParseString(&key)) {
                return false;
            }
            if (!Match(':')) {
                error = "expected ':' in object";
                return false;
            }
            JsonValue value;
            if (!ParseValue(&value)) {
                return false;
            }
            out->object_value.emplace(std::move(key), std::move(value));
            SkipWhitespace();
            if (Match('}')) {
                return true;
            }
            if (!Match(',')) {
                error = "expected ',' in object";
                return false;
            }
        }
    }

    bool ParseArray(JsonValue* out) {
        if (!Match('[')) {
            error = "expected '['";
            return false;
        }
        out->type = JsonValue::Type::kArray;
        out->array_value.clear();
        SkipWhitespace();
        if (Match(']')) {
            return true;
        }
        while (true) {
            JsonValue value;
            if (!ParseValue(&value)) {
                return false;
            }
            out->array_value.push_back(std::move(value));
            SkipWhitespace();
            if (Match(']')) {
                return true;
            }
            if (!Match(',')) {
                error = "expected ',' in array";
                return false;
            }
        }
    }

    bool ParseString(std::string* out) {
        if (!Match('"')) {
            error = "expected '\"'";
            return false;
        }
        std::string result;
        while (pos < input.size()) {
            char c = input[pos++];
            if (c == '"') {
                *out = std::move(result);
                return true;
            }
            if (c == '\\') {
                if (pos >= input.size()) {
                    error = "unterminated escape";
                    return false;
                }
                char esc = input[pos++];
                switch (esc) {
                    case '"': result.push_back('"'); break;
                    case '\\': result.push_back('\\'); break;
                    case '/': result.push_back('/'); break;
                    case 'b': result.push_back('\b'); break;
                    case 'f': result.push_back('\f'); break;
                    case 'n': result.push_back('\n'); break;
                    case 'r': result.push_back('\r'); break;
                    case 't': result.push_back('\t'); break;
                    case 'u':
                        error = "unicode escape not supported";
                        return false;
                    default:
                        error = "invalid escape";
                        return false;
                }
                continue;
            }
            result.push_back(c);
        }
        error = "unterminated string";
        return false;
    }

    bool ParseNumber(JsonValue* out) {
        size_t start = pos;
        if (input[pos] == '-') {
            pos++;
        }
        while (pos < input.size() && std::isdigit(static_cast<unsigned char>(input[pos]))) {
            pos++;
        }
        if (pos < input.size() && input[pos] == '.') {
            pos++;
            while (pos < input.size() && std::isdigit(static_cast<unsigned char>(input[pos]))) {
                pos++;
            }
        }
        if (pos < input.size() && (input[pos] == 'e' || input[pos] == 'E')) {
            pos++;
            if (pos < input.size() && (input[pos] == '+' || input[pos] == '-')) {
                pos++;
            }
            while (pos < input.size() && std::isdigit(static_cast<unsigned char>(input[pos]))) {
                pos++;
            }
        }
        const std::string_view slice(input.data() + start, pos - start);
        char* end_ptr = nullptr;
        const double value = std::strtod(slice.data(), &end_ptr);
        if (end_ptr == slice.data()) {
            error = "invalid number";
            return false;
        }
        out->type = JsonValue::Type::kNumber;
        out->number_value = value;
        return true;
    }

    bool ParseLiteral(JsonValue* out) {
        if (input.compare(pos, 4, "true") == 0) {
            pos += 4;
            out->type = JsonValue::Type::kBool;
            out->bool_value = true;
            return true;
        }
        if (input.compare(pos, 5, "false") == 0) {
            pos += 5;
            out->type = JsonValue::Type::kBool;
            out->bool_value = false;
            return true;
        }
        if (input.compare(pos, 4, "null") == 0) {
            pos += 4;
            out->type = JsonValue::Type::kNull;
            return true;
        }
        error = "invalid literal";
        return false;
    }
};
}  // namespace

const JsonValue* JsonValue::Get(const std::string& key) const {
    if (type != Type::kObject) {
        return nullptr;
    }
    auto it = object_value.find(key);
    if (it == object_value.end()) {
        return nullptr;
    }
    return &it->second;
}

const JsonValue* JsonValue::GetIndex(size_t index) const {
    if (type != Type::kArray || index >= array_value.size()) {
        return nullptr;
    }
    return &array_value[index];
}

bool ParseJson(const std::string& input, JsonValue* out_value, std::string* out_error) {
    if (!out_value) {
        return false;
    }
    Parser parser(input);
    if (!parser.ParseValue(out_value)) {
        if (out_error) {
            *out_error = parser.error;
        }
        return false;
    }
    parser.SkipWhitespace();
    if (parser.pos != input.size()) {
        if (out_error) {
            *out_error = "trailing characters";
        }
        return false;
    }
    return true;
}
