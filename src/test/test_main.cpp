#include "array.hpp"
#include "string.hpp"
#include "list.hpp"
#include "map.hpp"
#include "column.hpp"
#include "row.hpp"
#include "table.hpp"
#include "database.hpp"
#include "sql_parser.hpp"
#include "json_serializer.hpp"
#include "bplus_tree.hpp"
#include "exception.hpp"
#include "storage_engine.hpp"

#include <iostream>
#include <cassert>
#include <cstdio>

using namespace mini_db;

int tests_passed = 0;
int tests_failed = 0;

#define TEST(name) void name(); \
                   struct name##_register { name##_register() { tests.push_back(name); } } name##_instance; \
                   void name()

Array<void(*)()> tests;

void run_test(const char* name, void (*test_func)()) {
    std::cout << "Running: " << name << "..." << std::endl;
    try {
        test_func();
        std::cout << "  PASSED" << std::endl;
        ++tests_passed;
    } catch (const std::exception& e) {
        std::cout << "  FAILED: " << e.what() << std::endl;
        ++tests_failed;
    }
}

void test_array() {
    Array<int> arr;
    assert(arr.empty());
    assert(arr.size() == 0);

    arr.push_back(1);
    arr.push_back(2);
    arr.push_back(3);

    assert(arr.size() == 3);
    assert(arr[0] == 1);
    assert(arr[1] == 2);
    assert(arr[2] == 3);

    arr.pop_back();
    assert(arr.size() == 2);
    assert(arr.back() == 2);

    arr.clear();
    assert(arr.empty());

    Array<int> arr2;
    arr2.push_back(10);
    arr2.push_back(20);
    arr = arr2;
    assert(arr.size() == 2);
    assert(arr[0] == 10);
    assert(arr[1] == 20);
}

void test_string() {
    String str = "hello";
    assert(str.size() == 5);
    assert(str == "hello");

    str += " world";
    assert(str == "hello world");

    String sub = str.substr(0, 5);
    assert(sub == "hello");

    assert(str.find("world") == 6);
    assert(str.find("xyz") == String::npos);

    String lower = String("HELLO").to_lower();
    assert(lower == "hello");

    String upper = String("hello").to_upper();
    assert(upper == "HELLO");

    String trimmed = "  hello  ";
    trimmed.trim();
    assert(trimmed == "hello");

    assert(String("hello world").starts_with("hello"));
    assert(String("hello world").ends_with("world"));
    assert(!String("hello").starts_with("world"));
}

void test_list() {
    List<int> list;
    assert(list.empty());

    list.push_back(1);
    list.push_back(2);
    list.push_back(3);

    assert(list.size() == 3);
    assert(list.front() == 1);
    assert(list.back() == 3);

    list.pop_front();
    assert(list.front() == 2);

    list.clear();
    assert(list.empty());

    list.push_front(100);
    assert(list.front() == 100);
}

void test_map() {
    Map<String, int> map;
    assert(map.empty());

    map.insert("one", 1);
    map.insert("two", 2);
    map.insert("three", 3);

    assert(map.size() == 3);
    assert(*map.find("one") == 1);
    assert(*map.find("two") == 2);
    assert(map.find("four") == nullptr);

    map.remove("two");
    assert(map.size() == 2);
    assert(map.find("two") == nullptr);

    map["four"] = 4;
    assert(*map.find("four") == 4);

    auto keys = map.keys();
    assert(keys.size() == 3);
}

void test_bplus_tree() {
    BPlusTree<String, int> tree;
    assert(tree.empty());

    tree.insert("apple", 1);
    tree.insert("banana", 2);
    tree.insert("cherry", 3);

    assert(!tree.empty());
    assert(*tree.search("apple") == 1);
    assert(*tree.search("banana") == 2);
    assert(*tree.search("cherry") == 3);
    assert(tree.search("grape") == nullptr);

    tree.remove("banana");
    assert(tree.search("banana") == nullptr);
}

void test_column() {
    Column col1("id", ColumnType::INT, true);
    assert(col1.name() == "id");
    assert(col1.type() == ColumnType::INT);
    assert(col1.is_primary());

    Column col2("name", ColumnType::STRING, false);
    assert(col2.name() == "name");
    assert(col2.type() == ColumnType::STRING);
    assert(!col2.is_primary());

    assert(col1.type_string() == "int");
    assert(col2.type_string() == "string");
}

void test_row() {
    Array<Column> schema;
    schema.push_back(Column("id", ColumnType::INT, true));
    schema.push_back(Column("name", ColumnType::STRING));

    Row row(schema);
    row.set_value(0, "1001");
    row.set_value(1, "peter");

    assert(row.get_value(0) == "1001");
    assert(row.get_value(1) == "peter");
    assert(row.get_int_value(0) == 1001);
    assert(row.get_value("name") == "peter");
}

void test_table() {
    Array<Column> schema;
    schema.push_back(Column("id", ColumnType::INT, true));
    schema.push_back(Column("name", ColumnType::STRING));

    Table table("person", schema);

    Row row1(schema);
    row1.set_value(0, "1001");
    row1.set_value(1, "peter");
    table.insert(row1);

    Row row2(schema);
    row2.set_value(0, "1002");
    row2.set_value(1, "alice");
    table.insert(row2);

    auto results = table.select_all();
    assert(results.size() == 2);

    auto filtered = table.select("id", "=", "1001");
    assert(filtered.size() == 1);
    assert(filtered[0].get_value("name") == "peter");

    auto range = table.select("id", ">", "1000");
    assert(range.size() == 2);

    auto lt = table.select("id", "<", "1002");
    assert(lt.size() == 1);

    auto lte = table.select("id", "<=", "1001");
    assert(lte.size() == 1);

    auto gte = table.select("id", ">=", "1002");
    assert(gte.size() == 1);

    auto neq = table.select("id", "!=", "1001");
    assert(neq.size() == 1);
    assert(neq[0].get_value("name") == "alice");
}

void test_database() {
    Database db;

    db.create_database("testdb");
    db.use_database("testdb");

    Array<Column> schema;
    schema.push_back(Column("id", ColumnType::INT, true));
    schema.push_back(Column("name", ColumnType::STRING));

    db.create_table("person", schema);

    Table* table = db.get_table("person");
    assert(table != nullptr);
    assert(table->name() == "person");

    db.drop_table("person");

    try {
        db.get_table("person");
        assert(false);
    } catch (const Exception&) {
    }

    db.drop_database("testdb");
}

void test_sql_parser() {
    SQLParser parser;

    auto stmt1 = parser.parse("create database person");
    assert(stmt1.type == SQLType::CREATE_DATABASE);
    assert(stmt1.dbname == "person");

    auto stmt2 = parser.parse("use person");
    assert(stmt2.type == SQLType::USE);
    assert(stmt2.dbname == "person");

    auto stmt3 = parser.parse("create table person (id int primary, name string)");
    assert(stmt3.type == SQLType::CREATE_TABLE);
    assert(stmt3.tablename == "person");
    assert(stmt3.columns.size() == 2);

    auto stmt4 = parser.parse("insert person values(1001, \"peter\")");
    assert(stmt4.type == SQLType::INSERT);
    assert(stmt4.tablename == "person");
    assert(stmt4.values.size() == 2);

    auto stmt5 = parser.parse("select name from person where id = 1001");
    assert(stmt5.type == SQLType::SELECT);
    assert(stmt5.column_name == "name");
    assert(stmt5.tablename == "person");
    assert(stmt5.where.has_condition);
    assert(stmt5.where.column == "id");
    assert(stmt5.where.op == "=");
    assert(stmt5.where.value == "1001");

    auto stmt6 = parser.parse("delete person where id = 1001");
    assert(stmt6.type == SQLType::DELETE);
    assert(stmt6.where.has_condition);

    auto stmt7 = parser.parse("update person set name = \"john\" where id = 1001");
    assert(stmt7.type == SQLType::UPDATE);
    assert(stmt7.set_column == "name");
    assert(stmt7.set_value == "\"john\"");
}

void test_json_serializer() {
    String json = JsonSerializer::serialize_request("select * from person");
    assert(json.find("select") != String::npos);

    String resp = JsonSerializer::serialize_response(true, "OK");
    assert(resp.find("true") != String::npos);
    assert(resp.find("OK") != String::npos);
}

void test_storage_engine() {
    int ret = system("rm -rf test_data");
    (void)ret;

    Database db;
    StorageEngine engine("test_data");

    db.create_database("testdb");
    db.use_database("testdb");

    Array<Column> schema;
    schema.push_back(Column("id", ColumnType::INT, true));
    schema.push_back(Column("name", ColumnType::STRING));

    db.create_table("person", schema);

    Table* table = db.get_table("person");
    Row row1(schema);
    row1.set_value(0, "1");
    row1.set_value(1, "alice");
    table->insert(row1);

    Row row2(schema);
    row2.set_value(0, "2");
    row2.set_value(1, "bob");
    table->insert(row2);

    engine.save_database(db);

    auto table_files = engine.list_table_files("testdb");
    assert(table_files.size() >= 1);
    bool found_person = false;
    for (size_t i = 0; i < table_files.size(); ++i) {
        if (table_files[i] == "person") {
            found_person = true;
            break;
        }
    }
    assert(found_person);

    Table* loaded = engine.load_table("person", "testdb");
    assert(loaded != nullptr);
    assert(loaded->name() == "person");
    assert(loaded->select_all().size() == 2);

    auto results = loaded->select("name", "=", "alice");
    assert(results.size() == 1);
    assert(results[0].get_value("id") == "1");

    delete loaded;

    engine.drop_database_dir("testdb");
    ret = system("rm -rf test_data");
    (void)ret;
}

void test_sql_operators() {
    SQLParser parser;

    auto stmt1 = parser.parse("select * from t where id <= 10");
    assert(stmt1.where.op == "<=");

    auto stmt2 = parser.parse("select * from t where id >= 10");
    assert(stmt2.where.op == ">=");

    auto stmt3 = parser.parse("select * from t where id != 10");
    assert(stmt3.where.op == "!=");
}

void test_table_update_delete() {
    Array<Column> schema;
    schema.push_back(Column("id", ColumnType::INT, true));
    schema.push_back(Column("name", ColumnType::STRING));
    schema.push_back(Column("age", ColumnType::INT));

    Table table("person", schema);

    Row row1(schema);
    row1.set_value(0, "1");
    row1.set_value(1, "alice");
    row1.set_value(2, "25");
    table.insert(row1);

    Row row2(schema);
    row2.set_value(0, "2");
    row2.set_value(1, "bob");
    row2.set_value(2, "30");
    table.insert(row2);

    table.update_rows("name", "alice_updated", "id", "=", "1");
    auto results = table.select("name", "=", "alice_updated");
    assert(results.size() == 1);

    table.delete_rows("id", "=", "2");
    assert(table.select_all().size() == 1);

    table.clear();
    assert(table.select_all().size() == 0);
}

void test_exception() {
    try {
        throw Exception("test error");
    } catch (const Exception& e) {
        assert(e.message() == "test error");
    }
}

void test_map_copy() {
    Map<String, int> map1;
    map1["a"] = 1;
    map1["b"] = 2;

    Map<String, int> map2(map1);
    assert(*map2.find("a") == 1);
    assert(*map2.find("b") == 2);

    Map<String, int> map3;
    map3 = map1;
    assert(*map3.find("a") == 1);
}

void test_string_concat() {
    String s1 = "hello";
    String s2 = " world";
    String s3 = s1 + s2;
    assert(s3 == "hello world");

    String s4 = s1 + " test";
    assert(s4 == "hello test");
}

void test_edge_cases() {
    Array<Column> schema;
    schema.push_back(Column("id", ColumnType::INT, true));
    schema.push_back(Column("name", ColumnType::STRING));

    Table table("test", schema);

    try {
        Row bad_row(schema);
        bad_row.set_value(0, "1");
        bad_row.set_value(1, "test");
        bad_row.set_value(2, "extra");
        table.insert(bad_row);
        assert(false);
    } catch (const Exception& e) {
        assert(e.message().find("Column count mismatch") != String::npos);
    }

    Row row(schema);
    row.set_value(0, "1");
    row.set_value(1, "test");
    table.insert(row);

    try {
        Row dup_row(schema);
        dup_row.set_value(0, "1");
        dup_row.set_value(1, "duplicate");
        table.insert(dup_row);
        assert(false);
    } catch (const Exception& e) {
        assert(e.message().find("Duplicate primary key") != String::npos);
    }

    try {
        auto empty_results = table.select("nonexistent", "=", "value");
        assert(false);
    } catch (const Exception& e) {
        assert(e.message().find("Column not found") != String::npos);
    }

    SQLParser parser;
    try {
        parser.parse("create database ");
        assert(false);
    } catch (const Exception& e) {
        assert(e.message().find("Empty name") != String::npos);
    }

    auto empty_stmt = parser.parse("");
    assert(empty_stmt.type == SQLType::UNKNOWN);

    auto exit_stmt = parser.parse("exit");
    assert(exit_stmt.type == SQLType::EXIT);

    auto quit_stmt = parser.parse("quit");
    assert(quit_stmt.type == SQLType::EXIT);
}

void test_empty_table_operations() {
    Array<Column> schema;
    schema.push_back(Column("id", ColumnType::INT, true));
    schema.push_back(Column("name", ColumnType::STRING));

    Table table("empty", schema);

    auto results = table.select_all();
    assert(results.size() == 0);

    auto filtered = table.select("id", "=", "1");
    assert(filtered.size() == 0);

    bool deleted = table.delete_rows("id", "=", "1");
    assert(!deleted);

    bool updated = table.update_rows("name", "test", "id", "=", "1");
    assert(!updated);

    table.clear();
    assert(table.select_all().size() == 0);
}

void test_string_edge_cases() {
    String empty;
    assert(empty.empty());
    assert(empty.size() == 0);

    String only_spaces = "   ";
    only_spaces.trim();
    assert(only_spaces.empty());

    String s = "hello";
    assert(s.substr(0, 100) == "hello");
    assert(s.substr(5, 1).empty());

    assert(String("").empty());
    assert(String("a").find('a') == 0);
    assert(String("a").find('b') == String::npos);
}

void test_array_edge_cases() {
    Array<int> arr;
    assert(arr.empty());
    assert(arr.size() == 0);

    arr.push_back(1);
    assert(!arr.empty());

    arr.clear();
    assert(arr.empty());

    Array<int> arr2;
    arr2.push_back(10);
    arr2.push_back(20);

    Array<int> arr3(arr2);
    assert(arr3.size() == 2);
    assert(arr3[0] == 10);

    Array<int> arr4;
    arr4 = arr2;
    assert(arr4.size() == 2);
    assert(arr4[1] == 20);
}

int main() {
    std::cout << "=== MiniDB Unit Tests ===" << std::endl;
    std::cout << std::endl;

    run_test("Array", test_array);
    run_test("String", test_string);
    run_test("List", test_list);
    run_test("Map", test_map);
    run_test("BPlusTree", test_bplus_tree);
    run_test("Column", test_column);
    run_test("Row", test_row);
    run_test("Table", test_table);
    run_test("Database", test_database);
    run_test("SQLParser", test_sql_parser);
    run_test("JsonSerializer", test_json_serializer);
    run_test("StorageEngine", test_storage_engine);
    run_test("SQLOperators", test_sql_operators);
    run_test("TableUpdateDelete", test_table_update_delete);
    run_test("Exception", test_exception);
    run_test("MapCopy", test_map_copy);
    run_test("StringConcat", test_string_concat);
    run_test("EdgeCases", test_edge_cases);
    run_test("EmptyTableOperations", test_empty_table_operations);
    run_test("StringEdgeCases", test_string_edge_cases);
    run_test("ArrayEdgeCases", test_array_edge_cases);

    std::cout << std::endl;
    std::cout << "=== Test Results ===" << std::endl;
    std::cout << "Passed: " << tests_passed << std::endl;
    std::cout << "Failed: " << tests_failed << std::endl;
    std::cout << "Total: " << (tests_passed + tests_failed) << std::endl;

    return tests_failed > 0 ? 1 : 0;
}
