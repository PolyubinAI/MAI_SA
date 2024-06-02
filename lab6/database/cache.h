#pragma once

#include <Poco/Redis/Client.h>
#include <Poco/Redis/Redis.h>

#include <mutex>

namespace database {
namespace cache {
class Database {
private:
    Poco::Redis::Client _client;
    std::mutex _mtx;

    Database();

public:
    static Database& get();
    Poco::Redis::BulkString get(const std::string& key);
    void set(const std::string& key, const std::string& value, int expire);
};
}  // namespace cache
}  // namespace database