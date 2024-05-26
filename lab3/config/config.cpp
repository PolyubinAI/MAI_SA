#include "config.h"

#include <string>

std::string Config::getenv(const std::string&& s) {
    char* ptr = std::getenv(s.c_str());
    if (ptr) {
        return {ptr};
    }
    return {};
}

Config::Config() {
    _host = getenv("DB_HOST");
    _port = getenv("DB_PORT");
    _login = getenv("DB_LOGIN");
    _password = getenv("DB_PASSWORD");
    _database = getenv("DB_DATABASE");
}

Config& Config::get() {
    static Config _instance;
    return _instance;
}

const std::string& Config::get_port() const {
    return _port;
}

const std::string& Config::get_host() const {
    return _host;
}

const std::string& Config::get_login() const {
    return _login;
}

const std::string& Config::get_password() const {
    return _password;
}

const std::string& Config::get_database() const {
    return _database;
}

std::string& Config::database() {
    return _database;
}

std::string& Config::port() {
    return _port;
}

std::string& Config::host() {
    return _host;
}

std::string& Config::login() {
    return _login;
}

std::string& Config::password() {
    return _password;
}
