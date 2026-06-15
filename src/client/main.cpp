#include "tcp_client.hpp"
#include "json_serializer.hpp"
#include "logger.hpp"
#include "exception.hpp"
#include "array.hpp"
#include "string.hpp"

#include <iostream>
#include <sstream>
#include <chrono>
#include <iomanip>

using namespace mini_db;

void print_welcome() {
    std::cout << "Welcome to the MiniDB monitor. Commands end with ; or \\g." << std::endl;
    std::cout << "Your MiniDB connection id is 1" << std::endl;
    std::cout << "Server version: 1.0.0 MiniDB Server" << std::endl;
    std::cout << std::endl;
    std::cout << "Type 'help;' or '\\h' for help. Type '\\c' to clear the current input statement." << std::endl;
    std::cout << std::endl;
}

void print_usage() {
    std::cout << "For information about MiniDB products and services, please visit the project." << std::endl;
    std::cout << "Supported SQL commands:" << std::endl;
    std::cout << "  CREATE DATABASE <dbname>          - Create a new database" << std::endl;
    std::cout << "  DROP DATABASE <dbname>            - Drop a database" << std::endl;
    std::cout << "  USE <dbname>                      - Switch to a database" << std::endl;
    std::cout << "  CREATE TABLE <name> (...)         - Create a new table" << std::endl;
    std::cout << "  DROP TABLE <name>                 - Drop a table" << std::endl;
    std::cout << "  SELECT <col> FROM <table> [...]   - Query data" << std::endl;
    std::cout << "  INSERT <table> VALUES (...)       - Insert data" << std::endl;
    std::cout << "  UPDATE <table> SET ... [...]      - Update data" << std::endl;
    std::cout << "  DELETE <table> [...]              - Delete data" << std::endl;
    std::cout << "\nClient commands:" << std::endl;
    std::cout << "  help, \\h                         - Show this help" << std::endl;
    std::cout << "  exit, quit, \\q                   - Exit MiniDB" << std::endl;
    std::cout << "  clear, \\c                        - Clear input" << std::endl;
    std::cout << "  status, \\s                       - Show server status" << std::endl;
}

void print_table_result(const String& response) {
    // Extract success
    size_t pos = response.find("\"success\":");
    if (pos == String::npos) return;
    pos += 10;
    while (pos < response.size() && response[pos] == ' ') ++pos;
    bool success = (response[pos] == 't');

    // Extract message
    pos = response.find("\"message\":\"");
    if (pos == String::npos) return;
    pos += 12;
    size_t msg_end = response.find('\"', pos);
    if (msg_end == String::npos) return;
    String message = response.substr(pos, msg_end - pos);

    // Always print message for non-success
    if (!success) {
        std::cout << "ERROR: " << message.c_str() << std::endl;
        return;
    }

    // Extract rows
    pos = response.find("\"rows\":[");
    if (pos == String::npos) {
        std::cout << message.c_str() << std::endl;
        return;
    }
    pos += 8; // skip "rows":[

    // Check if rows is empty array
    while (pos < response.size() && response[pos] == ' ') ++pos;
    if (pos < response.size() && response[pos] == ']') {
        // No rows - print message (for DDL commands)
        std::cout << message.c_str() << std::endl;
        return;
    }

    // Parse rows - each row is {key:value,...}
    Array<Array<String>> rows_data;
    Array<String> headers;
    bool headers_set = false;

    pos = response.find('{', pos);
    while (pos < response.size() && pos != String::npos) {
        size_t obj_end = response.find('}', pos);
        if (obj_end == String::npos) break;

        String row_json = response.substr(pos + 1, obj_end - pos - 1);

        Array<String> row_values;
        Array<String> row_headers;

        size_t kpos = 0;
         while (kpos < row_json.size()) {
             kpos = row_json.find('\"', kpos);
             if (kpos == String::npos) break;
             ++kpos;
             size_t parse_pos = kpos;
             String key = JsonSerializer::parse_string_value(row_json, parse_pos);
             
             parse_pos = row_json.find(':', parse_pos);
             if (parse_pos == String::npos) break;
             ++parse_pos;
             String value = JsonSerializer::parse_value(row_json, parse_pos);
             kpos = parse_pos;

            row_headers.push_back(key);
            row_values.push_back(value);
        }

        if (!headers_set) {
            headers = row_headers;
            headers_set = true;
        }
        rows_data.push_back(row_values);

        pos = response.find('{', obj_end + 1);
    }

    if (headers.size() == 0) {
        std::cout << "Empty set" << std::endl;
        return;
    }

    Array<size_t> col_widths;
    for (size_t i = 0; i < headers.size(); ++i) {
        size_t width = headers[i].size();
        for (size_t j = 0; j < rows_data.size(); ++j) {
            if (i < rows_data[j].size()) {
                size_t val_len = rows_data[j][i].size();
                if (val_len > width) width = val_len;
            }
        }
        col_widths.push_back(width + 2);
    }

    auto print_separator = [&]() {
        std::cout << "+";
        for (size_t i = 0; i < col_widths.size(); ++i) {
            for (size_t j = 0; j < col_widths[i]; ++j) {
                std::cout << "-";
            }
            std::cout << "+";
        }
        std::cout << std::endl;
    };

    print_separator();

    std::cout << "|";
    for (size_t i = 0; i < headers.size(); ++i) {
        std::cout << " " << headers[i].c_str();
        size_t padding = col_widths[i] - headers[i].size() - 1;
        for (size_t j = 0; j < padding; ++j) std::cout << " ";
        std::cout << "|";
    }
    std::cout << std::endl;

    print_separator();

    for (size_t j = 0; j < rows_data.size(); ++j) {
        std::cout << "|";
        for (size_t i = 0; i < headers.size(); ++i) {
            String val = (i < rows_data[j].size()) ? rows_data[j][i] : "";
            std::cout << " " << val.c_str();
            size_t padding = col_widths[i] - val.size() - 1;
            for (size_t k = 0; k < padding; ++k) std::cout << " ";
            std::cout << "|";
        }
        std::cout << std::endl;
    }

    print_separator();

    std::cout << rows_data.size() << " row" << (rows_data.size() != 1 ? "s" : "") << " in set" << std::endl;
}

int main(int argc, char* argv[]) {
    String host = "127.0.0.1";
    int port = 8080;

    if (argc > 1) {
        host = argv[1];
    }
    if (argc > 2) {
        port = std::atoi(argv[2]);
    }

    Logger::instance().set_level(LogLevel::WARNING);

    std::cout << std::unitbuf;  // 禁用 stdout 缓冲，每次输出立即刷新
    std::cerr << std::unitbuf;  // 禁用 stderr 缓冲
    std::cin >> std::unitbuf;   // 禁用 stdin 缓冲

    std::cout << std::endl;
    print_welcome();

    TCPClient client(host, port);

    try {
        client.connect();
    } catch (const Exception& e) {
        std::cerr << "ERROR: Can't connect to server: " << e.what() << std::endl;
        return 1;
    }

    String input;
    char line[4096];
    String current_input;
    bool multiline = false;

    while (true) {
        if (multiline) {
            std::cout << "    -> ";
        } else {
            std::cout << "mini_db> ";
        }

        if (!std::cin.getline(line, sizeof(line))) {
            break;
        }

        String line_str = String(line);

        if (line_str == "\\c" || line_str == "clear") {
            current_input.clear();
            multiline = false;
            continue;
        }

        if (line_str == "\\h" || line_str == "help" || line_str == "help;") {
            print_usage();
            continue;
        }

        if (line_str == "\\q" || line_str == "exit" || line_str == "quit" ||
            line_str == "exit;" || line_str == "quit;") {
            String request = JsonSerializer::serialize_request("exit");
            try {
                client.send_request(request);
            } catch (...) {}
            break;
        }

        if (line_str == "\\s" || line_str == "status" || line_str == "status;") {
            std::cout << "Server: " << host.c_str() << ":" << port << std::endl;
            std::cout << "Connection: TCP/IP" << std::endl;
            std::cout << "Protocol version: 1.0" << std::endl;
            continue;
        }

        line_str.trim();
        if (line_str.empty()) continue;

        if (!current_input.empty()) {
            current_input += " " + line_str;
        } else {
            current_input = line_str;
        }

        bool ends_with_semicolon = (current_input.size() > 0 && current_input[current_input.size() - 1] == ';');

        if (!ends_with_semicolon) {
            multiline = true;
            continue;
        }

        String sql = current_input;
        if (ends_with_semicolon) {
            sql = current_input.substr(0, current_input.size() - 1);
        }
        current_input.clear();
        multiline = false;

        sql.trim();
        if (sql.empty()) continue;

        String request = JsonSerializer::serialize_request(sql);

        try {
            String response = client.send_request(request);
            print_table_result(response);
            std::cout.flush();
        } catch (const Exception& e) {
            std::cout << "ERROR: " << e.what() << std::endl;
            std::cout.flush();
        }

        std::cout << std::endl;
    }

    client.disconnect();
    std::cout << "Bye" << std::endl;

    return 0;
}
