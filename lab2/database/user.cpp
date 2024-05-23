#include "user.h"

#include <Poco/Data/PostgreSQL/PostgreSQLException.h>
#include <Poco/Data/RecordSet.h>
#include <Poco/Data/SessionFactory.h>
#include <Poco/Dynamic/Var.h>
#include <Poco/JSON/Parser.h>
#include <Poco/Crypto/DigestEngine.h>
#include <Poco/PBKDF2Engine.h>
#include <Poco/HMACEngine.h>
#include <Poco/SHA1Engine.h>

#include <exception>
#include <sstream>

#include "../config/config.h"
#include "database.h"

using namespace Poco::Data::Keywords;

namespace database {

Poco::JSON::Object::Ptr User::toJSON() const {
    Poco::JSON::Object::Ptr root = new Poco::JSON::Object();

    root->set("id", _id);
    root->set("first_name", _first_name);
    root->set("last_name", _last_name);
    root->set("email", _email);
    root->set("phone", _phone);
    root->set("login", _login);

    return root;
}

User User::fromJSON(const std::string& str) {
    User user;
    Poco::JSON::Parser parser;
    Poco::Dynamic::Var result = parser.parse(str);
    Poco::JSON::Object::Ptr object = result.extract<Poco::JSON::Object::Ptr>();

    user.id() = object->getValue<long>("id");
    user.first_name() = object->getValue<std::string>("first_name");
    user.last_name() = object->getValue<std::string>("last_name");
    user.email() = object->getValue<std::string>("email");
    user.phone() = object->getValue<std::string>("phone");
    user.login() = object->getValue<std::string>("login");
    user.password() = object->getValue<std::string>("password");

    return user;
}

std::optional<User> User::get_by_id(long id) {
    try {
        Poco::Data::Session session = database::Database::get().create_session();
        Poco::Data::Statement select(session);
        User a;
        select << "SELECT id, first_name, last_name, email, phone,login,password FROM users where id=$1",
            into(a._id),
            into(a._first_name),
            into(a._last_name),
            into(a._email),
            into(a._phone),
            into(a._login),
            into(a._password),
            use(id),
            range(0, 1);

        select.execute();
        Poco::Data::RecordSet rs(select);
        if (rs.moveFirst()) {
            return a;
        }
    }

    catch (Poco::Data::PostgreSQL::ConnectionException& e) {
        std::cerr << "connection:" << e.what() << std::endl;
    } catch (Poco::Data::PostgreSQL::StatementException& e) {
        std::cerr << "statement:" << e.what() << std::endl;
    }
    return {};
}

std::vector<User> User::search_by_name(std::string first_name, std::string last_name) {
    try {
        Poco::Data::Session session = database::Database::get().create_session();
        Poco::Data::Statement select(session);
        std::vector<User> result;
        User a;
        first_name += "%";
        last_name += "%";
        select << "SELECT id, first_name, last_name, email, phone, login, password FROM users where first_name LIKE $1 and last_name LIKE $2",
            into(a._id),
            into(a._first_name),
            into(a._last_name),
            into(a._email),
            into(a._phone),
            into(a._login),
            into(a._password),
            use(first_name),
            use(last_name),
            range(0, 1);

        while (!select.done()) {
            if (select.execute()) {
                result.push_back(a);
            }
        }
        return result;
    }

    catch (Poco::Data::PostgreSQL::ConnectionException& e) {
        std::cerr << "connection:" << e.what() << std::endl;
        throw;
    } catch (Poco::Data::PostgreSQL::StatementException& e) {
        std::cerr << "statement:" << e.what() << std::endl;
        throw;
    }
}

void User::update() {
    try {
        Poco::Data::Session session = database::Database::get().create_session();
        Poco::Data::Statement update(session);

        update << "UPDATE users SET first_name = $1, last_name = $2, email = $3, phone = $4, login = $5, password = $6 WHERE id = $7",
            use(_first_name),
            use(_last_name),
            use(_email),
            use(_phone),
            use(_login),
            use(_password),
            use(_id);

        update.execute();

        std::cerr << "updated:" << _id << std::endl;

    } catch (Poco::Data::PostgreSQL::ConnectionException& e) {
        std::cerr << "connection:" << e.what() << std::endl;
        throw;
    } catch (Poco::Data::PostgreSQL::StatementException& e) {
        std::cerr << "statement:" << e.what() << std::endl;
        throw;
    }
}

void User::save() {
    try {
        Poco::Data::Session session = database::Database::get().create_session();
        Poco::Data::Statement insert(session);

        insert << "INSERT INTO users (first_name,last_name,email,phone,login,password) VALUES($1, $2, $3, $4, $5, $6)",
            use(_first_name),
            use(_last_name),
            use(_email),
            use(_phone),
            use(_login),
            use(_password);

        insert.execute();

        Poco::Data::Statement select(session);
        select << "SELECT LASTVAL()",
            into(_id),
            range(0, 1);

        if (!select.done()) {
            select.execute();
        }
        std::cerr << "inserted:" << _id << std::endl;

    } catch (Poco::Data::PostgreSQL::ConnectionException& e) {
        std::cerr << "connection:" << e.what() << std::endl;
        throw;
    } catch (Poco::Data::PostgreSQL::StatementException& e) {
        std::cerr << "statement:" << e.what() << std::endl;
        throw;
    }
}

bool User::remove(long id) {
    try {
        Poco::Data::Session session = database::Database::get().create_session();
        Poco::Data::Statement remove(session);

        std::optional<User> exists = get_by_id(id);
        if (!exists) {
            return false;
        }

        remove << "DELETE FROM users WHERE id = $1",
            use(id);

        remove.execute();

        std::cerr << "deleted:" << id << std::endl;

    } catch (Poco::Data::PostgreSQL::ConnectionException& e) {
        std::cerr << "connection:" << e.what() << std::endl;
        throw;
    } catch (Poco::Data::PostgreSQL::StatementException& e) {
        std::cerr << "statement:" << e.what() << std::endl;
        throw;
    }
    return true;
}

bool User::check_name(std::string& reason) {
    if (_first_name.find('\t') != std::string::npos) {
        reason = "Name can't contain spaces";
        return false;
    }

    return true;
}

bool User::check_email(std::string& reason) {
    if (_email.find('@') == std::string::npos) {
        reason = "Email must contain @";
        return false;
    }

    if (_email.find(' ') != std::string::npos) {
        reason = "EMail can't contain spaces";
        return false;
    }

    if (_email.find('\t') != std::string::npos) {
        reason = "EMail can't contain spaces";
        return false;
    }

    return true;
}

void User::hash_password() {
    Poco::PBKDF2Engine<Poco::HMACEngine<Poco::SHA1Engine>> pbkdf2("salt", 4096, 128);
    pbkdf2.update(_password);
    _password = Poco::DigestEngine::digestToHex(pbkdf2.digest());
}

const std::string& User::get_login() const {
    return _login;
}

const std::string& User::get_password() const {
    return _password;
}

std::string& User::login() {
    return _login;
}

std::string& User::password() {
    return _password;
}

long User::get_id() const {
    return _id;
}

const std::string& User::get_first_name() const {
    return _first_name;
}

const std::string& User::get_last_name() const {
    return _last_name;
}

const std::string& User::get_email() const {
    return _email;
}

const std::string& User::get_phone() const {
    return _phone;
}

long& User::id() {
    return _id;
}

std::string& User::first_name() {
    return _first_name;
}

std::string& User::last_name() {
    return _last_name;
}

std::string& User::email() {
    return _email;
}

std::string& User::phone() {
    return _phone;
}
}  // namespace database
