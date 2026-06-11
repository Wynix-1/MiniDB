#pragma once

#include "string.hpp"
#include "array.hpp"
#include "row.hpp"
#include "table.hpp"

namespace mini_db {

class JsonSerializer {
public:
    static String serialize_string(const String& str) {
        String result = "\"";
        for (size_t i = 0; i < str.size(); ++i) {
            char c = str[i];
            if (c == '"') {
                result += "\\\"";
            } else if (c == '\\') {
                result += "\\\\";
            } else if (c == '\n') {
                result += "\\n";
            } else if (c == '\r') {
                result += "\\r";
            } else if (c == '\t') {
                result += "\\t";
            } else {
                result.push_back(c);
            }
        }
        result += "\"";
        return result;
    }

    static String serialize_int(int value) {
        char buf[32];
        std::snprintf(buf, sizeof(buf), "%d", value);
        return String(buf);
    }

    static String serialize_bool(bool value) {
        return value ? "true" : "false";
    }

    static String serialize_null() {
        return "null";
    }

    static String serialize_object_begin() { return "{"; }
    static String serialize_object_end() { return "}"; }
    static String serialize_array_begin() { return "["; }
    static String serialize_array_end() { return "]"; }
    static String serialize_comma() { return ","; }
    static String serialize_colon() { return ":"; }

    static String serialize_key_value(const String& key, const String& value, bool last = false) {
        String result = serialize_string(key) + serialize_colon() + value;
        if (!last) {
            result += serialize_comma();
        }
        return result;
    }

    static String serialize_row(const Row& row) {
        String result = serialize_object_begin();
        for (size_t i = 0; i < row.values().size(); ++i) {
            String key = row.schema()[i].name();
            String value;

            if (row.schema()[i].type() == ColumnType::STRING) {
                value = serialize_string(row.get_value(i));
            } else {
                value = serialize_string(row.get_value(i));
            }

            result += serialize_key_value(key, value, i == row.values().size() - 1);
        }
        result += serialize_object_end();
        return result;
    }

    static String serialize_rows(const Array<Row>& rows) {
        String result = serialize_array_begin();
        for (size_t i = 0; i < rows.size(); ++i) {
            result += serialize_row(rows[i]);
            if (i < rows.size() - 1) {
                result += serialize_comma();
            }
        }
        result += serialize_array_end();
        return result;
    }

    static String serialize_response(bool success, const String& message, const Array<Row>& rows) {
        String result = serialize_object_begin();
        result += serialize_key_value("success", serialize_bool(success), false);
        result += serialize_key_value("message", serialize_string(message), false);
        result += serialize_key_value("rows", serialize_rows(rows), true);
        result += serialize_object_end();
        return result;
    }

    static String serialize_response(bool success, const String& message) {
        Array<Row> empty_rows;
        return serialize_response(success, message, empty_rows);
    }

    static String serialize_request(const String& sql) {
        String result = serialize_object_begin();
        result += serialize_key_value("sql", serialize_string(sql), true);
        result += serialize_object_end();
        return result;
    }

    static String parse_string_value(const String& json, size_t& pos) {
        String result;
        while (pos < json.size() && json[pos] == ' ') ++pos;

        if (pos >= json.size() || json[pos] != '"') {
            return result;
        }
        ++pos;

        while (pos < json.size() && json[pos] != '"') {
            if (json[pos] == '\\' && pos + 1 < json.size()) {
                ++pos;
                switch (json[pos]) {
                    case '"': result.push_back('"'); break;
                    case '\\': result.push_back('\\'); break;
                    case 'n': result.push_back('\n'); break;
                    case 'r': result.push_back('\r'); break;
                    case 't': result.push_back('\t'); break;
                    default: result.push_back(json[pos]); break;
                }
            } else {
                result.push_back(json[pos]);
            }
            ++pos;
        }

        if (pos < json.size()) ++pos;
        return result;
    }

    static String parse_value(const String& json, size_t& pos) {
        while (pos < json.size() && json[pos] == ' ') ++pos;

        if (pos >= json.size()) return "";

        if (json[pos] == '"') {
            return parse_string_value(json, pos);
        }

        String result;
        while (pos < json.size() && json[pos] != ',' && json[pos] != '}' && json[pos] != ']') {
            result.push_back(json[pos]);
            ++pos;
        }
        result.trim();
        return result;
    }

    static bool parse_bool(const String& json, size_t& pos) {
        String value = parse_value(json, pos);
        return value == "true";
    }
};

} // namespace mini_db
