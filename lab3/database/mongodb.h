#pragma once

#include <Poco/JSON/Object.h>
#include <Poco/MongoDB/Connection.h>
#include <Poco/MongoDB/Cursor.h>
#include <Poco/MongoDB/Database.h>
#include <Poco/MongoDB/Document.h>
#include <Poco/MongoDB/MongoDB.h>

#include <optional>
#include <string>
#include <vector>

namespace database {
namespace mongodb {
std::string getId();

class Database {
private:
    Poco::MongoDB::Connection _connection;
    Poco::MongoDB::Database _database;

    Database();

public:
    static Database& get();

    static void JSONToDocument(Poco::JSON::Object::Ptr json, Poco::MongoDB::Document& doc);
    void save(const std::string& collection, Poco::JSON::Object::Ptr json);
    void update(const std::string& collection, std::string id, Poco::JSON::Object::Ptr json);
    std::vector<std::string> select(const std::string& collection, Poco::MongoDB::Document& doc);
    std::optional<std::string> select(const std::string& collection, std::string id);
    bool remove(const std::string& collection, std::string id);
};
}  // namespace mongodb
}  // namespace database
