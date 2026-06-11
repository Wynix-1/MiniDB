#pragma once

#include "table.hpp"
#include "string.hpp"
#include "map.hpp"
#include "exception.hpp"
#include "logger.hpp"

namespace mini_db {

class Database {
public:
    Database() : current_db_("") {}

    void create_database(const String& dbname) {
        if (databases_.contains(dbname)) {
            throw Exception("Database already exists: " + dbname);
        }
        databases_[dbname] = Map<String, Table>();
        LOG_INFO("Database created: " + dbname);
    }

    void drop_database(const String& dbname) {
        if (!databases_.contains(dbname)) {
            throw Exception("Database not found: " + dbname);
        }
        databases_.remove(dbname);
        if (current_db_ == dbname) {
            current_db_ = "";
        }
        LOG_INFO("Database dropped: " + dbname);
    }

    void use_database(const String& dbname) {
        if (!databases_.contains(dbname)) {
            throw Exception("Database not found: " + dbname);
        }
        current_db_ = dbname;
        LOG_INFO("Using database: " + dbname);
    }

    void create_table(const String& tablename, const Array<Column>& columns) {
        check_current_db();

        auto* db = databases_.find(current_db_);
        if (!db) throw Exception("Database not found: " + current_db_);

        if (db->contains(tablename)) {
            throw Exception("Table already exists: " + tablename);
        }

        (*db)[tablename] = Table(tablename, columns);
        LOG_INFO("Table created: " + tablename);
    }

    void drop_table(const String& tablename) {
        check_current_db();

        auto* db = databases_.find(current_db_);
        if (!db) throw Exception("Database not found: " + current_db_);

        if (!db->contains(tablename)) {
            throw Exception("Table not found: " + tablename);
        }

        db->remove(tablename);
        LOG_INFO("Table dropped: " + tablename);
    }

    Table* get_table(const String& tablename) {
        check_current_db();

        auto* db = databases_.find(current_db_);
        if (!db) throw Exception("Database not found: " + current_db_);

        auto* table = db->find(tablename);
        if (!table) {
            throw Exception("Table not found: " + tablename);
        }

        return table;
    }

    void register_table(const String& tablename, Table* table) {
        check_current_db();

        auto* db = databases_.find(current_db_);
        if (!db) throw Exception("Database not found: " + current_db_);

        (*db)[tablename] = *table;
        delete table;
        LOG_INFO("Table registered: " + tablename);
    }

    const String& current_database() const { return current_db_; }

    Array<String> list_databases() const {
        Array<String> result;
        auto keys = databases_.keys();
        for (size_t i = 0; i < keys.size(); ++i) {
            result.push_back(keys[i]);
        }
        return result;
    }

    Array<String> list_tables() const {
        Array<String> result;
        if (current_db_.empty()) return result;

        auto* db = databases_.find(current_db_);
        if (!db) return result;

        auto keys = db->keys();
        for (size_t i = 0; i < keys.size(); ++i) {
            result.push_back(keys[i]);
        }
        return result;
    }

private:
    void check_current_db() const {
        if (current_db_.empty()) {
            throw Exception("No database selected. Use 'use <dbname>' first.");
        }
    }

    String current_db_;
    Map<String, Map<String, Table>> databases_;
};

} // namespace mini_db
