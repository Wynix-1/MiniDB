#pragma once

#include "database.hpp"
#include "string.hpp"
#include "exception.hpp"
#include "logger.hpp"
#include <fstream>
#include <sstream>
#include <cstdio>
#include <cstdlib>
#include <sys/stat.h>

namespace mini_db {

class StorageEngine {
public:
    StorageEngine(const String& data_dir = "data") : data_dir_(data_dir) {
        create_directory(data_dir_);
    }

    void save_database(Database& db) {
        if (db.current_database().empty()) {
            throw Exception("No database selected");
        }

        String db_dir = data_dir_ + "/" + db.current_database();
        create_directory(db_dir);

        auto tables = db.list_tables();
        for (size_t i = 0; i < tables.size(); ++i) {
            Table* table = db.get_table(tables[i]);
            if (table) {
                save_table(*table, db.current_database());
            }
        }
    }

    void load_database(Database& db, const String& dbname) {
        String db_dir = data_dir_ + "/" + dbname;

        if (!directory_exists(db_dir)) {
            return;
        }

        db.use_database(dbname);
        LOG_INFO("Database loaded: " + dbname);
    }

    void save_table(const Table& table, const String& dbname) {
        String db_dir = data_dir_ + "/" + dbname;
        create_directory(db_dir);
        
        String table_file = db_dir + "/" + table.name() + ".dat";

        std::ofstream file(table_file.c_str());
        if (!file.is_open()) {
            throw Exception("Cannot open file for writing: " + table_file);
        }

        file << "MINIDB_V1" << "\n";

        file << table.columns().size() << "\n";
        for (size_t i = 0; i < table.columns().size(); ++i) {
            const auto& col = table.columns()[i];
            write_line(file, col.name().c_str());
            write_line(file, col.type_string().c_str());
            write_line(file, col.is_primary() ? "1" : "0");
        }

        file << table.rows().size() << "\n";
        for (size_t i = 0; i < table.rows().size(); ++i) {
            const auto& row = table.rows()[i];
            for (size_t j = 0; j < row.values().size(); ++j) {
                write_line(file, row.values()[j].c_str());
            }
        }

        file.close();
        LOG_INFO("Table saved: " + table.name() + " (" + 
                 String(std::to_string(table.rows().size()).c_str()) + " rows)");
    }

    Table* load_table(const String& tablename, const String& dbname) {
        String table_file = data_dir_ + "/" + dbname + "/" + tablename + ".dat";

        if (!file_exists(table_file)) {
            return nullptr;
        }

        std::ifstream file(table_file.c_str());
        if (!file.is_open()) {
            throw Exception("Cannot open file for reading: " + table_file);
        }

        char header_buf[64];
        file.getline(header_buf, sizeof(header_buf));
        String header(header_buf);
        if (header != "MINIDB_V1") {
            throw Exception("Invalid data file format: " + table_file);
        }

        Array<Column> columns;
        size_t col_count = read_number(file);

        for (size_t i = 0; i < col_count; ++i) {
            String name = read_line(file);
            String type_str = read_line(file);
            String primary_str = read_line(file);

            ColumnType type = (type_str == "string") ? ColumnType::STRING : ColumnType::INT;
            bool is_primary = (primary_str == "1");
            columns.push_back(Column(name, type, is_primary));
        }

        Table* table = new Table(tablename, columns);

        size_t row_count = read_number(file);

        for (size_t i = 0; i < row_count; ++i) {
            Row row(columns);
            for (size_t j = 0; j < columns.size(); ++j) {
                String value = read_line(file);
                row.set_value(j, value);
            }
            table->insert(row);
        }

        file.close();
        LOG_INFO("Table loaded: " + tablename + " (" + 
                 String(std::to_string(row_count).c_str()) + " rows)");

        return table;
    }

    void delete_table_files(const String& tablename, const String& dbname) {
        String dat_file = data_dir_ + "/" + dbname + "/" + tablename + ".dat";
        remove_file(dat_file);
        LOG_INFO("Table files deleted: " + tablename);
    }

    bool database_exists(const String& dbname) const {
        return directory_exists(data_dir_ + "/" + dbname);
    }

    void create_database_dir(const String& dbname) {
        create_directory(data_dir_ + "/" + dbname);
    }

    void drop_database_dir(const String& dbname) {
        String db_dir = data_dir_ + "/" + dbname;
        String cmd = "rm -rf " + db_dir;
        int ret = system(cmd.c_str());
        (void)ret;
        LOG_INFO("Database directory removed: " + dbname);
    }

    Array<String> list_table_files(const String& dbname) const {
        Array<String> tables;
        String db_dir = data_dir_ + "/" + dbname;

        if (!directory_exists(db_dir)) {
            return tables;
        }

        String cmd = "ls " + db_dir + "/*.dat 2>/dev/null";
        FILE* pipe = popen(cmd.c_str(), "r");
        if (pipe) {
            char buffer[256];
            while (fgets(buffer, sizeof(buffer), pipe)) {
                String line(buffer);
                line.trim();
                if (!line.empty()) {
                    size_t last_slash = String::npos;
                    for (size_t k = 0; k < line.size(); ++k) {
                        if (line[k] == '/') last_slash = k;
                    }
                    if (last_slash != String::npos) {
                        String filename = line.substr(last_slash + 1);
                        size_t dot_pos = filename.find(".dat");
                        if (dot_pos != String::npos) {
                            tables.push_back(filename.substr(0, dot_pos));
                        }
                    }
                }
            }
            pclose(pipe);
        }

        return tables;
    }

private:
    String data_dir_;

    static void write_line(std::ofstream& file, const char* str) {
        file << str << "\n";
    }

    static String read_line(std::ifstream& file) {
        String line;
        char buffer[512];
        file.getline(buffer, sizeof(buffer));
        return String(buffer);
    }

    static size_t read_number(std::ifstream& file) {
        size_t num;
        file >> num;
        char c;
        while (file.get(c) && c != '\n');
        return num;
    }

    bool file_exists(const String& path) const {
        std::ifstream file(path.c_str());
        return file.good();
    }

    bool directory_exists(const String& path) const {
        struct stat info;
        return stat(path.c_str(), &info) == 0 && (info.st_mode & S_IFDIR);
    }

    void create_directory(const String& path) {
        String cmd = "mkdir -p " + path;
        int ret = system(cmd.c_str());
        (void)ret;
    }

    void remove_file(const String& path) {
        remove(path.c_str());
    }
};

} // namespace mini_db
