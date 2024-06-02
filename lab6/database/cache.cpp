#include "cache.h"

#include <Poco/Redis/Array.h>
#include <Poco/Redis/Client.h>
#include <Poco/Redis/Command.h>
#include <Poco/Redis/Redis.h>

#include <mutex>

#include "../config/config.h"

namespace database {
namespace cache {
Database::Database() {
    std::cout << "Connecting to redis: " << Config::get().get_host() << ":" << Config::get().get_port() << std::endl;
    _client.connect(Config::get().get_host(), std::stoi(Config::get().get_port()));
}

Database& Database::get() {
    static Database _instance;
    return _instance;
}

Poco::Redis::BulkString Database::get(const std::string& key) {
    std::lock_guard<std::mutex> lck(_mtx);
    Poco::Redis::Command cmd = Poco::Redis::Command::get(key);
    return _client.execute<Poco::Redis::BulkString>(static_cast<Poco::Redis::Array>(cmd));
}

void Database::set(const std::string& key, const std::string& value, int expire) {
    std::lock_guard<std::mutex> lck(_mtx);
    Poco::Redis::Command cmd = Poco::Redis::Command::set(key, value, true, {expire, 0});
    _client.sendCommand(static_cast<Poco::Redis::Array>(cmd));
}
}  // namespace cache
}  // namespace database