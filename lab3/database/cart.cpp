#include "cart.h"

#include <Poco/JSON/Object.h>
#include <Poco/JSON/Parser.h>

#include <optional>
#include <string>
#include <vector>
#include <algorithm> 

#include "mongodb.h"
#include "item.h"

using database::mongodb::Database;

namespace database {
const std::string Cart::collection = "carts";

Cart Cart::fromJSON(const std::string& str) {
    Cart cart;
    Poco::JSON::Parser parser;
    Poco::Dynamic::Var result = parser.parse(str);
    Poco::JSON::Object::Ptr object = result.extract<Poco::JSON::Object::Ptr>();
    cart.id() = object->getValue<std::string>("_id");
    for (auto& var: *object->getArray("items")) {
        cart.items().push_back(var.extract<std::string>());
    }
    return cart;
}

std::optional<Cart> Cart::get_by_id(std::string id) {
    std::optional<std::string> result = Database::get().select(collection, id);
    if (result) {
        return {fromJSON(result.value())};
    }
    return {};
}

void Cart::create(std::string id) {
    Database::get().save(collection, Cart(id).toJSON());
}

bool Cart::remove(std::string id) {
    return Database::get().remove(collection, id);
}

void Cart::add_item(std::string id) {
    _items.push_back(id);
    Poco::JSON::Object::Ptr push = new Poco::JSON::Object(), element = new Poco::JSON::Object();
    element->set("items", id);
    push->set("$push", element);
    Database::get().update(collection, _id, push);
}

bool Cart::remove_item(std::string id) {
    bool deleted = false;
    for (auto it = _items.begin(); it != _items.end(); ++it) {
        if (*it == id) {
            deleted = true;
            _items.erase(it);
            break;
        }
    }
    if (deleted) {
        Database::get().update(collection, _id, toJSON());
    }
    return deleted;
}

Poco::JSON::Object::Ptr Cart::toJSON() const {
    Poco::JSON::Object::Ptr root = new Poco::JSON::Object();

    root->set("_id", _id);
    Poco::JSON::Array::Ptr items = new Poco::JSON::Array();
    for (auto item: _items) {
        items->add(item);
    }
    root->set("items", items);

    return root;
}
}