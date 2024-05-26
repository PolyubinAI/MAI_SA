#pragma once

#include <Poco/JSON/Object.h>

#include <optional>
#include <string>
#include <vector>

namespace database {
class User {
private:
    long _id;
    std::string _first_name;
    std::string _last_name;
    std::string _email;
    std::string _phone;
    std::string _login;
    std::string _password;

public:
    static User fromJSON(const std::string& str);

    long get_id() const { return _id; };
    const std::string& get_first_name() const { return _first_name; };
    const std::string& get_last_name() const { return _last_name; };
    const std::string& get_email() const { return _email; };
    const std::string& get_phone() const { return _phone; };
    const std::string& get_login() const { return _login; };
    const std::string& get_password() const { return _password; };

    long& id() { return _id; };
    std::string& first_name() { return _first_name; };
    std::string& last_name() { return _last_name; };
    std::string& email() { return _email; };
    std::string& phone() { return _phone; };
    std::string& login() { return _login; };
    std::string& password() { return _password; };

    bool check_name(std::string& reason);
    bool check_email(std::string& reason);
    void hash_password();

    static std::optional<User> get_by_id(long id);
    static std::vector<User> search_by_name(std::string first_name, std::string last_name);
    static bool remove(long id);
    void update();
    void save();

    Poco::JSON::Object::Ptr toJSON() const;
};
}  // namespace database
