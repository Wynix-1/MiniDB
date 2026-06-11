#pragma once

#include "column.hpp"
#include "array.hpp"
#include "string.hpp"

namespace mini_db {

class Row {
public:
    Row() : values_(), columns_schema_() {}

    Row(const Array<Column>& schema) : columns_schema_(schema) {
        values_.resize(schema.size());
    }

    void set_value(size_t index, const String& value) {
        if (index >= values_.size()) {
            values_.resize(index + 1);
        }
        values_[index] = value;
    }

    void set_value(const String& column_name, const String& value) {
        for (size_t i = 0; i < columns_schema_.size(); ++i) {
            if (columns_schema_[i].name() == column_name) {
                set_value(i, value);
                return;
            }
        }
    }

    String get_value(size_t index) const {
        if (index < values_.size()) {
            return values_[index];
        }
        return "";
    }

    String get_value(const String& column_name) const {
        for (size_t i = 0; i < columns_schema_.size(); ++i) {
            if (columns_schema_[i].name() == column_name) {
                return get_value(i);
            }
        }
        return "";
    }

    int get_int_value(size_t index) const {
        if (index < values_.size()) {
            return std::atoi(values_[index].c_str());
        }
        return 0;
    }

    int get_int_value(const String& column_name) const {
        for (size_t i = 0; i < columns_schema_.size(); ++i) {
            if (columns_schema_[i].name() == column_name) {
                return get_int_value(i);
            }
        }
        return 0;
    }

    size_t size() const { return values_.size(); }

    const Array<Column>& schema() const { return columns_schema_; }
    void set_schema(const Array<Column>& schema) { columns_schema_ = schema; }

    const Array<String>& values() const { return values_; }
    Array<String>& values() { return values_; }

    String to_string() const {
        String result = "(";
        for (size_t i = 0; i < values_.size(); ++i) {
            if (i > 0) result += ", ";
            if (columns_schema_.size() > i && columns_schema_[i].type() == ColumnType::STRING) {
                result += "\"" + values_[i] + "\"";
            } else {
                result += values_[i];
            }
        }
        result += ")";
        return result;
    }

private:
    Array<String> values_;
    Array<Column> columns_schema_;
};

} // namespace mini_db
