#pragma once

#include "string.hpp"
#include "array.hpp"
#include "column.hpp"

namespace mini_db {

enum class SQLType {
    CREATE_DATABASE,
    DROP_DATABASE,
    USE,
    CREATE_TABLE,
    DROP_TABLE,
    SELECT,
    INSERT,
    UPDATE,
    DELETE,
    EXIT,
    UNKNOWN
};

struct WhereCondition {
    String column;
    String op;
    String value;
    bool has_condition = false;
};

struct SQLStatement {
    SQLType type;
    String dbname;
    String tablename;
    String column_name;
    Array<Column> columns;
    Array<String> values;
    String set_column;
    String set_value;
    WhereCondition where;
    String raw_sql;
};

class SQLParser {
public:
    SQLStatement parse(const String& sql) {
        SQLStatement stmt;
        stmt.raw_sql = sql;

        String trimmed = sql;
        trimmed.trim();

        if (trimmed.empty()) {
            stmt.type = SQLType::UNKNOWN;
            return stmt;
        }

        if (trimmed == "exit" || trimmed == "quit") {
            stmt.type = SQLType::EXIT;
            return stmt;
        }

        String upper = trimmed.to_upper();

        if (upper.starts_with("CREATE DATABASE")) {
            stmt.type = SQLType::CREATE_DATABASE;
            stmt.dbname = extract_name(trimmed, 15);
        } else if (upper.starts_with("DROP DATABASE")) {
            stmt.type = SQLType::DROP_DATABASE;
            stmt.dbname = extract_name(trimmed, 13);
        } else if (upper.starts_with("USE ")) {
            stmt.type = SQLType::USE;
            stmt.dbname = extract_name(trimmed, 4);
        } else if (upper.starts_with("CREATE TABLE")) {
            stmt.type = SQLType::CREATE_TABLE;
            parse_create_table(trimmed, stmt);
        } else if (upper.starts_with("DROP TABLE")) {
            stmt.type = SQLType::DROP_TABLE;
            stmt.tablename = extract_name(trimmed, 10);
        } else if (upper.starts_with("SELECT")) {
            stmt.type = SQLType::SELECT;
            parse_select(trimmed, stmt);
        } else if (upper.starts_with("INSERT")) {
            stmt.type = SQLType::INSERT;
            parse_insert(trimmed, stmt);
        } else if (upper.starts_with("UPDATE")) {
            stmt.type = SQLType::UPDATE;
            parse_update(trimmed, stmt);
        } else if (upper.starts_with("DELETE")) {
            stmt.type = SQLType::DELETE;
            parse_delete(trimmed, stmt);
        } else {
            stmt.type = SQLType::UNKNOWN;
        }

        return stmt;
    }

private:
    String extract_name(const String& sql, size_t start_pos) {
        String result;
        for (size_t i = start_pos; i < sql.size(); ++i) {
            char c = sql[i];
            if (c != ' ' && c != '\t') {
                result.push_back(c);
            } else if (!result.empty()) {
                break;
            }
        }
        result.trim();
        if (result.empty()) {
            throw Exception("Empty name specified");
        }
        return result.to_lower();
    }

    void parse_create_table(const String& sql, SQLStatement& stmt) {
        size_t table_start = 12;
        while (sql[table_start] == ' ' || sql[table_start] == '\t') ++table_start;

        size_t table_end = table_start;
        while (table_end < sql.size() && sql[table_end] != '(') ++table_end;

        stmt.tablename = sql.substr(table_start, table_end - table_start).to_lower();
        stmt.tablename.trim();

        size_t cols_start = sql.find('(');
        size_t cols_end = sql.find(')');

        if (cols_start == String::npos || cols_end == String::npos) {
            throw Exception("Invalid CREATE TABLE syntax");
        }

        String cols_str = sql.substr(cols_start + 1, cols_end - cols_start - 1);

        size_t pos = 0;
        while (pos < cols_str.size()) {
            String col_def;
            while (pos < cols_str.size() && cols_str[pos] != ',') {
                col_def.push_back(cols_str[pos]);
                ++pos;
            }
            ++pos;

            col_def.trim();
            if (col_def.empty()) continue;

            Array<String> parts;
            String current;
            for (size_t i = 0; i < col_def.size(); ++i) {
                if (col_def[i] == ' ' || col_def[i] == '\t') {
                    if (!current.empty()) {
                        parts.push_back(current);
                        current.clear();
                    }
                } else {
                    current.push_back(col_def[i]);
                }
            }
            if (!current.empty()) {
                parts.push_back(current);
            }

            if (parts.size() >= 2) {
                String col_name = parts[0].to_lower();
                ColumnType type = ColumnType::INT;
                bool is_primary = false;

                if (parts[1].to_lower() == "string") {
                    type = ColumnType::STRING;
                }

                if (parts.size() >= 3 && parts[2].to_lower() == "primary") {
                    is_primary = true;
                }

                stmt.columns.push_back(Column(col_name, type, is_primary));
            }
        }
    }

    void parse_select(const String& sql, SQLStatement& stmt) {
        size_t from_pos = sql.to_upper().find(" FROM ");
        size_t where_pos = sql.to_upper().find(" WHERE ");

        if (from_pos == String::npos) {
            throw Exception("Invalid SELECT syntax: missing FROM");
        }

        String col_str = sql.substr(7, from_pos - 7);
        col_str.trim();
        stmt.column_name = col_str.to_lower();

        size_t table_start = from_pos + 6;
        size_t table_end = (where_pos != String::npos) ? where_pos : sql.size();
        String table_str = sql.substr(table_start, table_end - table_start);
        table_str.trim();
        stmt.tablename = table_str.to_lower();

        if (where_pos != String::npos) {
            parse_where(sql.substr(where_pos + 7), stmt.where);
        }
    }

    void parse_insert(const String& sql, SQLStatement& stmt) {
        String upper = sql.to_upper();
        size_t values_pos = upper.find("VALUES");
        if (values_pos == String::npos) {
            throw Exception("Invalid INSERT syntax: missing VALUES");
        }

        String table_str = sql.substr(6, values_pos - 6);
        table_str.trim();
        stmt.tablename = table_str.to_lower();

        size_t paren_start = sql.find('(', values_pos);
        size_t paren_end = sql.find(')', paren_start);

        if (paren_start == String::npos || paren_end == String::npos) {
            throw Exception("Invalid INSERT syntax: missing parentheses");
        }

        String values_str = sql.substr(paren_start + 1, paren_end - paren_start - 1);

        String current;
        bool in_quotes = false;
        for (size_t i = 0; i < values_str.size(); ++i) {
            char c = values_str[i];
            if (c == '"') {
                in_quotes = !in_quotes;
            } else if (c == ',' && !in_quotes) {
                current.trim();
                stmt.values.push_back(current);
                current.clear();
            } else {
                current.push_back(c);
            }
        }
        if (!current.empty()) {
            current.trim();
            stmt.values.push_back(current);
        }
    }

    void parse_update(const String& sql, SQLStatement& stmt) {
        size_t set_pos = sql.to_upper().find(" SET ");
        size_t where_pos = sql.to_upper().find(" WHERE ");

        if (set_pos == String::npos) {
            throw Exception("Invalid UPDATE syntax: missing SET");
        }

        String table_str = sql.substr(7, set_pos - 7);
        table_str.trim();
        stmt.tablename = table_str.to_lower();

        size_t set_start = set_pos + 5;
        size_t set_end = (where_pos != String::npos) ? where_pos : sql.size();
        String set_str = sql.substr(set_start, set_end - set_start);
        set_str.trim();

        size_t eq_pos = set_str.find('=');
        if (eq_pos == String::npos) {
            throw Exception("Invalid UPDATE syntax: missing =");
        }

        stmt.set_column = set_str.substr(0, eq_pos).to_lower();
        stmt.set_column.trim();

        stmt.set_value = set_str.substr(eq_pos + 1);
        stmt.set_value.trim();

        if (where_pos != String::npos) {
            parse_where(sql.substr(where_pos + 7), stmt.where);
        }
    }

    void parse_delete(const String& sql, SQLStatement& stmt) {
        String upper = sql.to_upper();
        size_t from_pos = upper.find(" FROM ");
        size_t where_pos = upper.find(" WHERE ");

        if (from_pos != String::npos) {
            size_t table_start = from_pos + 6;
            size_t table_end = (where_pos != String::npos) ? where_pos : sql.size();
            String table_str = sql.substr(table_start, table_end - table_start);
            table_str.trim();
            stmt.tablename = table_str.to_lower();
        } else {
            size_t table_start = 7;
            size_t table_end = (where_pos != String::npos) ? where_pos : sql.size();
            String table_str = sql.substr(table_start, table_end - table_start);
            table_str.trim();
            stmt.tablename = table_str.to_lower();
        }

        if (where_pos != String::npos) {
            parse_where(sql.substr(where_pos + 7), stmt.where);
        }
    }

    void parse_where(const String& sql, WhereCondition& where) {
        String trimmed = sql;
        trimmed.trim();

        String ops[] = {"<=", ">=", "!=", "=", "<", ">"};
        for (const auto& op : ops) {
            size_t op_pos = trimmed.find(op);
            if (op_pos != String::npos) {
                where.column = trimmed.substr(0, op_pos).to_lower();
                where.column.trim();
                where.op = op;
                where.value = trimmed.substr(op_pos + op.size());
                where.value.trim();
                where.has_condition = true;
                return;
            }
        }
    }
};

} // namespace mini_db
