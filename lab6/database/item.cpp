#include "item.h"

#include <Poco/JSON/Object.h>
#include <Poco/JSON/Parser.h>

#include <optional>
#include <string>
#include <vector>

#include "mongodb.h"

using database::mongodb::Database;

namespace database {
const std::string Item::collection = "items";

Item Item::fromJSON(const std::string& str) {
    Item item;
    Poco::JSON::Parser parser;
    Poco::Dynamic::Var result = parser.parse(str);
    Poco::JSON::Object::Ptr object = result.extract<Poco::JSON::Object::Ptr>();

    item.id() = object->getValue<std::string>("_id");
    item.name() = object->getValue<std::string>("name");
    item.price() = object->getValue<double>("price");

    return item;
}

bool Item::check_price(std::string& reason) {
    if (_price < 0) {
        reason = "Price must be above zero";
        return false;
    }
    return true;
}

std::optional<Item> Item::get_by_id(std::string id) {
    std::optional<std::string> result = Database::get().select(collection, id);
    if (result) {
        return {fromJSON(result.value())};
    }
    return {};
}

std::vector<Item> Item::get_all() {
    std::vector<Item> result;
    for (auto& str : Database::get().select_all(collection)) {
        result.push_back(fromJSON(str));
    }
    return result;
}

std::vector<Item> Item::search_by_name(std::string name) {
    std::vector<Item> result;
    Poco::MongoDB::Document selector;
    selector.add("name", name);
    for (auto& str : Database::get().select(collection, selector)) {
        result.push_back(fromJSON(str));
    }
    return result;
}

bool Item::remove(std::string id) {
    return Database::get().remove(collection, id);
}

void Item::update() {
    Database::get().update(collection, _id, toJSON());
}

void Item::save() {
    Database::get().save(collection, toJSON());
}

Poco::JSON::Object::Ptr Item::toJSON() const {
    Poco::JSON::Object::Ptr root = new Poco::JSON::Object();

    root->set("_id", _id);
    root->set("name", _name);
    root->set("price", _price);

    return root;
}
}  // namespace database
