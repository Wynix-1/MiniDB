#pragma once

#include "string.hpp"

namespace mini_db {

enum class ColumnType {
    INT,
    STRING
};

class Column {
public:
    Column() : type_(ColumnType::INT), is_primary_(false) {}

    Column(const String& name, ColumnType type, bool is_primary = false)
        : name_(name), type_(type), is_primary_(is_primary) {}

    const String& name() const { return name_; }
    ColumnType type() const { return type_; }
    bool is_primary() const { return is_primary_; }

    String type_string() const {
        switch (type_) {
            case ColumnType::INT: return "int";
            case ColumnType::STRING: return "string";
            default: return "unknown";
        }
    }

private:
    String name_;
    ColumnType type_;
    bool is_primary_;
};

} // namespace mini_db
