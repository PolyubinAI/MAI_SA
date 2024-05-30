#include "postgres.h"

#include <Poco/Data/PostgreSQL/Connector.h>
#include <Poco/Data/SessionPool.h>
#include <Poco/JSON/Object.h>

#include <iostream>

#include "../config/config.h"

namespace database {
namespace postgres {
Database::Database() {
    _connection_string += "host=";
    _connection_string += Config::get().get_host();
    _connection_string += " user=";
    _connection_string += Config::get().get_login();
    _connection_string += " dbname=";
    _connection_string += Config::get().get_database();
    _connection_string += " port=";
    _connection_string += Config::get().get_port();
    _connection_string += " password=";
    _connection_string += Config::get().get_password();

    std::cout << "Connecting to postgres:" << _connection_string << std::endl;
    Poco::Data::PostgreSQL::Connector::registerConnector();
    _pool = std::make_unique<Poco::Data::SessionPool>(Poco::Data::PostgreSQL::Connector::KEY, _connection_string);
}

Database& Database::get() {
    static Database _instance;
    return _instance;
}

Poco::Data::Session Database::create_session() {
    return Poco::Data::Session(_pool->get());
}
}  // namespace postgres
}