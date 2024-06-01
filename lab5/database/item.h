#pragma once

#include <Poco/JSON/Object.h>

#include <optional>
#include <string>
#include <vector>

namespace database {
class Item {
private:
    std::string _id;
    std::string _name;
    double _price;

public:
    static const std::string collection;

    Item() = default;
    Item(std::string id)
        : _id(id) {}
    Item(std::string id, std::string name, double price)
        : _id(id), _name(name), _price(price) {}

    static Item fromJSON(const std::string& str);

    const std::string& get_id() const { return _id; };
    const std::string& get_name() const { return _name; };
    double get_price() const { return _price; };

    std::string& id() { return _id; };
    std::string& name() { return _name; };
    double& price() { return _price; };

    bool check_price(std::string& reason);

    static std::optional<Item> get_by_id(std::string id);
    static std::vector<Item> get_all();
    static std::vector<Item> search_by_name(std::string name);
    static bool remove(std::string id);
    void update();
    void save();

    Poco::JSON::Object::Ptr toJSON() const;
};
}  // namespace database
