#pragma once

#include <Poco/JSON/Object.h>

#include <optional>
#include <string>
#include <vector>

#include "item.h"

namespace database {
class Cart {
private:
    std::string _id;
    std::vector<std::string> _items;

public:
    static const std::string collection;

    Cart() = default;
    Cart(std::string id)
        : _id(id) {}

    static Cart fromJSON(const std::string& str);

    const std::string& get_id() const { return _id; };
    const std::vector<std::string>& get_items() const { return _items; };

    std::string& id() { return _id; };
    std::vector<std::string>& items() { return _items; };

    static std::optional<Cart> get_by_id(std::string id);
    static void create(std::string id);
    static bool remove(std::string id);
    void add_item(std::string id);
    bool remove_item(std::string id);

    Poco::JSON::Object::Ptr toJSON() const;
};
}  // namespace database
