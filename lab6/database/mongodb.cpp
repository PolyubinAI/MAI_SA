#include "mongodb.h"

#include <Poco/Data/PostgreSQL/Connector.h>
#include <Poco/Data/SessionPool.h>
#include <Poco/JSON/Object.h>
#include <Poco/MongoDB/Array.h>
#include <Poco/MongoDB/Document.h>

#include <ctime>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>

#include "../config/config.h"

namespace database {
namespace mongodb {
std::string getId() {
    static uint32_t counter = std::random_device{}();

    std::ostringstream os;
    os << std::setfill('0') << std::setw(8) << std::hex << std::time(nullptr)
       << std::setfill('0') << std::setw(8) << std::hex << counter++;

    return os.str();
}

Database::Database()
    : _database(Config::get().get_database()) {
    std::cout << "Connecting to mongodb: " << Config::get().get_host() << ":" << Config::get().get_port() << std::endl;
    _connection.connect(Config::get().get_host(), std::stoi(Config::get().get_port()));
}

void Database::JSONToDocument(Poco::JSON::Object::Ptr json, Poco::MongoDB::Document& doc) {
    for (auto& item : *json) {
        if (item.second.isInteger()) {
            doc.add(item.first, item.second.extract<int>());
        } else if (item.second.isNumeric()) {
            doc.add(item.first, item.second.extract<double>());
        } else if (item.second.isArray()) {
            auto& arr = doc.addNewArray(item.first);
            for (auto& v : *item.second.extract<Poco::JSON::Array::Ptr>()) {
                arr.add(v.toString());
            }
        } else if (item.second.isString()) {
            doc.add(item.first, item.second.extract<std::string>());
        } else {
            try {
                Poco::JSON::Object::Ptr child = item.second.extract<Poco::JSON::Object::Ptr>();
                Poco::MongoDB::Document& child_doc = doc.addNewDocument(item.first);
                JSONToDocument(child, child_doc);
            } catch (...) {
                doc.add(item.first, item.second.toString());
            }
        }
    }
}

Database& Database::get() {
    static Database _instance;
    return _instance;
}

void Database::save(const std::string& collection, Poco::JSON::Object::Ptr json) {
    try {
        Poco::SharedPtr<Poco::MongoDB::InsertRequest> request = _database.createInsertRequest(collection);

        Poco::MongoDB::Document& doc = request->addNewDocument();
        JSONToDocument(json, doc);
        _connection.sendRequest(*request);

    } catch (std::exception& ex) {
        std::cout << "mongodb exception: " << ex.what() << std::endl;
        std::string last_error = _database.getLastError(_connection);
        if (!last_error.empty()) {
            std::cout << "mongodb last error: " << last_error << std::endl;
        }
    }
}

void Database::update(const std::string& collection, std::string id, Poco::JSON::Object::Ptr json) {
    try {
        Poco::SharedPtr<Poco::MongoDB::UpdateRequest> request = _database.createUpdateRequest(collection);

        request->selector().add("_id", id);
        Poco::MongoDB::Document& doc = request->update();
        JSONToDocument(json, doc);
        _connection.sendRequest(*request);

    } catch (std::exception& ex) {
        std::cout << "mongodb exception: " << ex.what() << std::endl;
        std::string last_error = _database.getLastError(_connection);
        if (!last_error.empty()) {
            std::cout << "mongodb last error: " << last_error << std::endl;
        }
    }
}

std::vector<std::string> Database::select(const std::string& collection, Poco::MongoDB::Document& selector) {
    std::vector<std::string> result;
    try {
        Poco::SharedPtr<Poco::MongoDB::QueryRequest> request = _database.createQueryRequest(collection);
        Poco::MongoDB::ResponseMessage response;

        request->selector() = selector;
        _connection.sendRequest(*request, response);
        for (auto& doc : response.documents()) {
            result.push_back(doc->toString());
        }

    } catch (std::exception& ex) {
        std::cout << "mongodb exception: " << ex.what() << std::endl;
        std::string last_error = _database.getLastError(_connection);
        if (!last_error.empty()) {
            std::cout << "mongodb last error: " << last_error << std::endl;
        }
    }
    return result;
}

std::optional<std::string> Database::select(const std::string& collection, std::string id) {
    Poco::MongoDB::Document selector;
    selector.add("_id", id);
    std::vector<std::string> result = select(collection, selector);
    if (result.empty()) {
        return {};
    }
    return {result.front()};
}

std::vector<std::string> Database::select_all(const std::string& collection) {
    std::vector<std::string> result;
    try {
        Poco::SharedPtr<Poco::MongoDB::QueryRequest> request = _database.createQueryRequest(collection);
        Poco::MongoDB::ResponseMessage response;

        _connection.sendRequest(*request, response);
        for (auto& doc : response.documents()) {
            result.push_back(doc->toString());
        }

    } catch (std::exception& ex) {
        std::cout << "mongodb exception: " << ex.what() << std::endl;
        std::string last_error = _database.getLastError(_connection);
        if (!last_error.empty()) {
            std::cout << "mongodb last error: " << last_error << std::endl;
        }
    }
    return result;
}

bool Database::remove(const std::string& collection, std::string id) {
    std::optional<std::string> exists = select(collection, id);
    if (exists) {
        try {
            Poco::SharedPtr<Poco::MongoDB::DeleteRequest> request = _database.createDeleteRequest(collection);

            request->selector().add("_id", id);
            _connection.sendRequest(*request);
            return true;
        } catch (std::exception& ex) {
            std::cout << "mongodb exception: " << ex.what() << std::endl;
            std::string last_error = _database.getLastError(_connection);
            if (!last_error.empty()) {
                std::cout << "mongodb last error: " << last_error << std::endl;
            }
        }
    }
    return false;
}
}  // namespace mongodb
}  // namespace database
