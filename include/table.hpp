#pragma once

#include "row.hpp"
#include "array.hpp"
#include "string.hpp"
#include "bplus_tree.hpp"
#include "exception.hpp"
#include <cstdlib>

namespace mini_db {

class Table {
public:
    Table() : name_(""), primary_key_index_(-1) {}

    Table(const String& name, const Array<Column>& columns)
        : name_(name), columns_(columns), primary_key_index_(-1) {
        for (size_t i = 0; i < columns.size(); ++i) {
            if (columns[i].is_primary()) {
                primary_key_index_ = static_cast<int>(i);
                break;
            }
        }
    }

    const String& name() const { return name_; }
    const Array<Column>& columns() const { return columns_; }
    const Array<Row>& rows() const { return rows_; }
    int primary_key_index() const { return primary_key_index_; }

    void insert(const Row& row) {
        if (row.values().size() != columns_.size()) {
            throw Exception("Column count mismatch: expected " + 
                           String(std::to_string(columns_.size()).c_str()) + 
                           ", got " + String(std::to_string(row.values().size()).c_str()));
        }

        if (primary_key_index_ >= 0) {
            String pk_value = row.get_value(primary_key_index_);
            if (pk_value.empty()) {
                throw Exception("Primary key value cannot be empty");
            }
            if (index_.search(pk_value) != nullptr) {
                throw Exception("Duplicate primary key: " + pk_value);
            }
            index_.insert(pk_value, static_cast<int>(rows_.size()));
        }
        rows_.push_back(row);
    }

    Array<Row> select(const String& column_name, const String& op, const String& value) const {
        Array<Row> results;
        int col_index = -1;

        for (size_t i = 0; i < columns_.size(); ++i) {
            if (columns_[i].name() == column_name) {
                col_index = static_cast<int>(i);
                break;
            }
        }

        if (col_index < 0) {
            throw Exception("Column not found: " + column_name);
        }

        if (primary_key_index_ == col_index && op == "=") {
            int* row_index = index_.search(value);
            if (row_index && *row_index >= 0 && *row_index < static_cast<int>(rows_.size())) {
                results.push_back(rows_[*row_index]);
            }
            return results;
        }

        for (size_t i = 0; i < rows_.size(); ++i) {
            String cell_value = rows_[i].get_value(col_index);
            bool match = compare_values(cell_value, value, op, columns_[col_index].type());

            if (match) {
                results.push_back(rows_[i]);
            }
        }

        return results;
    }

    Array<Row> select_all() const {
        return rows_;
    }

    bool delete_rows(const String& column_name, const String& op, const String& value) {
        int col_index = -1;
        for (size_t i = 0; i < columns_.size(); ++i) {
            if (columns_[i].name() == column_name) {
                col_index = static_cast<int>(i);
                break;
            }
        }

        if (col_index < 0) {
            throw Exception("Column not found: " + column_name);
        }

        Array<Row> new_rows;
        bool deleted = false;

        for (size_t i = 0; i < rows_.size(); ++i) {
            String cell_value = rows_[i].get_value(col_index);
            bool match = compare_values(cell_value, value, op, columns_[col_index].type());

            if (!match) {
                new_rows.push_back(rows_[i]);
            } else {
                deleted = true;
            }
        }

        if (deleted) {
            rows_ = new_rows;
            rebuild_index();
        }

        return deleted;
    }

    bool update_rows(const String& column_name, const String& value,
                     const String& where_column, const String& where_op, const String& where_value) {
        int col_index = -1;
        for (size_t i = 0; i < columns_.size(); ++i) {
            if (columns_[i].name() == column_name) {
                col_index = static_cast<int>(i);
                break;
            }
        }

        if (col_index < 0) {
            throw Exception("Column not found: " + column_name);
        }

        int where_col_index = -1;
        if (!where_column.empty()) {
            for (size_t i = 0; i < columns_.size(); ++i) {
                if (columns_[i].name() == where_column) {
                    where_col_index = static_cast<int>(i);
                    break;
                }
            }
        }

        bool updated = false;

        for (size_t i = 0; i < rows_.size(); ++i) {
            bool match = (where_col_index < 0);

            if (!match) {
                String cell_value = rows_[i].get_value(where_col_index);
                match = compare_values(cell_value, where_value, where_op, columns_[where_col_index].type());
            }

            if (match) {
                rows_[i].set_value(col_index, value);
                updated = true;
            }
        }

        if (updated && primary_key_index_ >= 0) {
            rebuild_index();
        }

        return updated;
    }

    void clear() {
        rows_.clear();
        index_ = BPlusTree<String, int>();
    }

private:
    static bool compare_values(const String& cell, const String& target, 
                               const String& op, ColumnType type) {
        if (type == ColumnType::INT) {
            int a = std::atoi(cell.c_str());
            int b = std::atoi(target.c_str());
            if (op == "=") return a == b;
            if (op == "<") return a < b;
            if (op == ">") return a > b;
            if (op == "<=") return a <= b;
            if (op == ">=") return a >= b;
            if (op == "!=") return a != b;
        } else {
            if (op == "=") return cell == target;
            if (op == "<") return cell < target;
            if (op == ">") return cell > target;
            if (op == "<=") return cell <= target;
            if (op == ">=") return cell >= target;
            if (op == "!=") return cell != target;
        }
        return false;
    }

    void rebuild_index() {
        if (primary_key_index_ < 0) return;

        index_ = BPlusTree<String, int>();
        for (size_t i = 0; i < rows_.size(); ++i) {
            String pk_value = rows_[i].get_value(primary_key_index_);
            index_.insert(pk_value, static_cast<int>(i));
        }
    }

    String name_;
    Array<Column> columns_;
    Array<Row> rows_;
    int primary_key_index_;
    BPlusTree<String, int> index_;
};

} // namespace mini_db
