#include "database.hpp"
#include "sql_parser.hpp"
#include "storage_engine.hpp"
#include "tcp_server.hpp"
#include "json_serializer.hpp"
#include "logger.hpp"
#include "exception.hpp"

#include <iostream>
#include <csignal>
#include <atomic>

using namespace mini_db;

static std::atomic<bool> g_running(true);
static Database* g_db = nullptr;
static StorageEngine* g_storage = nullptr;

void signal_handler(int signum) {
    LOG_INFO("Received signal " + String(std::to_string(signum).c_str()) + ", shutting down...");

    if (g_db && g_storage && !g_db->current_database().empty()) {
        try {
            g_storage->save_database(*g_db);
            LOG_INFO("Database saved before shutdown");
        } catch (const Exception& e) {
            LOG_ERROR("Failed to save database: " + e.message());
        }
    }

    g_running = false;
}

int main(int argc, char* argv[]) {
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    int port = 8080;
    if (argc > 1) {
        port = std::atoi(argv[1]);
    }

    Logger::instance().set_level(LogLevel::INFO);
    LOG_INFO("MiniDB Server starting...");

    Database db;
    SQLParser parser;
    StorageEngine storage("data");

    g_db = &db;
    g_storage = &storage;

    TCPServer server(port);

    auto handler = [&](const String& request) -> String {
        try {
            size_t pos = 0;
            // Parse JSON: {"sql":"..."} - find the "sql" key first
            size_t sql_key_pos = request.find("\"sql\"");
            if (sql_key_pos == String::npos) {
                return JsonSerializer::serialize_response(false, "Invalid request format");
            }
            // Move past "sql" and the colon
            pos = sql_key_pos + 5; // skip "sql"
            while (pos < request.size() && request[pos] != ':') ++pos;
            ++pos; // skip ':'
            String sql = JsonSerializer::parse_string_value(request, pos);

            SQLStatement stmt = parser.parse(sql);

            switch (stmt.type) {
                case SQLType::CREATE_DATABASE:
                    db.create_database(stmt.dbname);
                    storage.create_database_dir(stmt.dbname);
                    return JsonSerializer::serialize_response(true, "Query OK, database created");

                case SQLType::DROP_DATABASE:
                    db.drop_database(stmt.dbname);
                    storage.drop_database_dir(stmt.dbname);
                    return JsonSerializer::serialize_response(true, "Query OK, database dropped");

                case SQLType::USE:
                    db.use_database(stmt.dbname);
                    if (storage.database_exists(stmt.dbname)) {
                        auto table_files = storage.list_table_files(stmt.dbname);
                        for (size_t i = 0; i < table_files.size(); ++i) {
                            Table* table = storage.load_table(table_files[i], stmt.dbname);
                            if (table) {
                                db.register_table(table_files[i], table);
                            }
                        }
                    }
                    return JsonSerializer::serialize_response(true, "Database changed");

                case SQLType::CREATE_TABLE:
                    db.create_table(stmt.tablename, stmt.columns);
                    storage.save_table(*db.get_table(stmt.tablename), db.current_database());
                    return JsonSerializer::serialize_response(true, "Query OK, table created");

                case SQLType::DROP_TABLE:
                    db.drop_table(stmt.tablename);
                    storage.delete_table_files(stmt.tablename, db.current_database());
                    return JsonSerializer::serialize_response(true, "Query OK, table dropped");

                case SQLType::SELECT: {
                    Table* table = db.get_table(stmt.tablename);
                    Array<Row> results;

                    if (stmt.column_name == "*") {
                        if (stmt.where.has_condition) {
                            results = table->select(stmt.where.column, stmt.where.op, stmt.where.value);
                        } else {
                            results = table->select_all();
                        }
                    } else {
                        Array<Row> all_rows;
                        if (stmt.where.has_condition) {
                            all_rows = table->select(stmt.where.column, stmt.where.op, stmt.where.value);
                        } else {
                            all_rows = table->select_all();
                        }

                        for (size_t i = 0; i < all_rows.size(); ++i) {
                            Row filtered_row(table->columns());
                            for (size_t j = 0; j < table->columns().size(); ++j) {
                                if (table->columns()[j].name() == stmt.column_name) {
                                    filtered_row.set_value(j, all_rows[i].get_value(j));
                                }
                            }
                            results.push_back(filtered_row);
                        }
                    }

                    return JsonSerializer::serialize_response(true, "Query OK", results);
                }

                case SQLType::INSERT: {
                    Table* table = db.get_table(stmt.tablename);
                    Row row(table->columns());

                    for (size_t i = 0; i < stmt.values.size() && i < table->columns().size(); ++i) {
                        row.set_value(i, stmt.values[i]);
                    }

                    table->insert(row);
                    storage.save_table(*table, db.current_database());
                    return JsonSerializer::serialize_response(true, "Query OK, 1 row affected");
                }

                case SQLType::UPDATE: {
                    Table* table = db.get_table(stmt.tablename);
                    table->update_rows(
                        stmt.set_column, stmt.set_value,
                        stmt.where.column, stmt.where.op, stmt.where.value
                    );
                    storage.save_table(*table, db.current_database());
                    return JsonSerializer::serialize_response(true, "Query OK, rows updated");
                }

                case SQLType::DELETE: {
                    Table* table = db.get_table(stmt.tablename);

                    if (stmt.where.has_condition) {
                        table->delete_rows(stmt.where.column, stmt.where.op, stmt.where.value);
                    } else {
                        table->clear();
                    }

                    storage.save_table(*table, db.current_database());
                    return JsonSerializer::serialize_response(true, "Query OK, rows deleted");
                }

                case SQLType::EXIT:
                    return JsonSerializer::serialize_response(true, "Goodbye");

                default:
                    return JsonSerializer::serialize_response(false, "Unknown command: " + sql);
            }
        } catch (const Exception& e) {
            return JsonSerializer::serialize_response(false, e.message());
        } catch (...) {
            return JsonSerializer::serialize_response(false, "Unknown error occurred");
        }
    };

    std::thread server_thread([&]() {
        server.start(handler);
    });

    LOG_INFO("Server is running on port " + String(std::to_string(port).c_str()) + ". Press Ctrl+C to stop.");

    while (g_running) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    server.stop();
    server_thread.join();

    if (!db.current_database().empty()) {
        try {
            storage.save_database(db);
            LOG_INFO("Database saved on shutdown");
        } catch (const Exception& e) {
            LOG_ERROR("Failed to save database: " + e.message());
        }
    }

    LOG_INFO("Server stopped.");
    return 0;
}
