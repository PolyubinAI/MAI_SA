#include "item_handler.h"

#include <Poco/Exception.h>
#include <Poco/Net/HTMLForm.h>
#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/URI.h>

#include <fstream>
#include <iostream>
#include <string>

#include "../../database/mongodb.h"
#include "../../database/item.h"

void ItemHandler::handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response) {
    Poco::Net::HTMLForm form(request, request.stream());
    Poco::URI uri(request.getURI());

    response.setChunkedTransferEncoding(true);
    response.setContentType("application/json");

    try {
        if (uri.getPath() == "/item") {
            if (request.getMethod() == Poco::Net::HTTPRequest::HTTP_GET) {
                std::string id = form.get("id");

                std::optional<database::Item> result = database::Item::get_by_id(id);
                if (result) {
                    response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                    std::ostream& ostr = response.send();
                    Poco::JSON::Stringifier::stringify(result->toJSON(), ostr);
                    return;
                } else {
                    response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_NOT_FOUND);
                    Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
                    root->set("status", static_cast<int>(response.getStatus()));
                    root->set("detail", "item not found");
                    root->set("instance", uri.getPath());
                    std::ostream& ostr = response.send();
                    Poco::JSON::Stringifier::stringify(root, ostr);
                    return;
                }
            } else if (request.getMethod() == Poco::Net::HTTPRequest::HTTP_POST) {
                std::string message;
                if (form.has("name") && form.has("price")) {
                    database::Item item(database::mongodb::getId());
                    item.name() = form.get("name");
                    item.price() = std::stod(form.get("price"));

                    bool valid_request = true;
                    std::string reason;

                    if (!item.check_price(reason)) {
                        valid_request = false;
                        message += reason;
                        message += "\n";
                    }

                    if (valid_request) {
                        item.save();
                        response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                        std::ostream& ostr = response.send();
                        Poco::JSON::Stringifier::stringify(item.toJSON(), ostr);
                        return;
                    }
                }

                if (message.empty()) {
                    message = "item information is incomplete";
                }

                response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_BAD_REQUEST);
                Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
                root->set("status", static_cast<int>(response.getStatus()));
                root->set("detail", message);
                root->set("instance", uri.getPath());
                std::ostream& ostr = response.send();
                Poco::JSON::Stringifier::stringify(root, ostr);
                return;
            } else if (request.getMethod() == Poco::Net::HTTPRequest::HTTP_PUT) {
                std::string id = form.get("id");

                std::optional<database::Item> result = database::Item::get_by_id(id);
                if (result) {
                    database::Item& item = result.value();
                    item.name() = form.get("name", item.name());
                    item.price() = std::stod(form.get("price"));

                    bool valid_request = true;
                    std::string reason, message;

                    if (!item.check_price(reason)) {
                        valid_request = false;
                        message += reason;
                        message += "\n";
                    }

                    if (valid_request) {
                        item.save();
                        response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                        std::ostream& ostr = response.send();
                        Poco::JSON::Stringifier::stringify(item.toJSON(), ostr);
                        return;
                    }

                    response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_BAD_REQUEST);
                    Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
                    root->set("status", static_cast<int>(response.getStatus()));
                    root->set("detail", message);
                    root->set("instance", uri.getPath());
                    std::ostream& ostr = response.send();
                    Poco::JSON::Stringifier::stringify(root, ostr);
                    return;
                }

                response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_NOT_FOUND);
                Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
                root->set("status", static_cast<int>(response.getStatus()));
                root->set("detail", "item not found");
                root->set("instance", uri.getPath());
                std::ostream& ostr = response.send();
                Poco::JSON::Stringifier::stringify(root, ostr);
                return;
            } else if (request.getMethod() == Poco::Net::HTTPRequest::HTTP_DELETE) {
                std::string id = form.get("id");

                if (database::Item::remove(id)) {
                    response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                    response.send();
                    return;
                } else {
                    response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_NOT_FOUND);
                    Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
                    root->set("status", static_cast<int>(response.getStatus()));
                    root->set("detail", "item not found");
                    root->set("instance", uri.getPath());
                    std::ostream& ostr = response.send();
                    Poco::JSON::Stringifier::stringify(root, ostr);
                    return;
                }
            }
        } else if (uri.getPath() == "/item/all" && request.getMethod() == Poco::Net::HTTPRequest::HTTP_GET) {
            std::vector<database::Item> items = database::Item::get_all();
            Poco::JSON::Array result;
            for (auto s : items) {
                result.add(s.toJSON());
            }
            response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
            std::ostream& ostr = response.send();
            Poco::JSON::Stringifier::stringify(result, ostr);
            return;
        } else if (uri.getPath() == "/item/search" && request.getMethod() == Poco::Net::HTTPRequest::HTTP_GET) {
            std::string name = form.get("name");
            std::vector<database::Item> items = database::Item::search_by_name(name);
            Poco::JSON::Array result;
            for (auto s : items) {
                result.add(s.toJSON());
            }
            response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
            std::ostream& ostr = response.send();
            Poco::JSON::Stringifier::stringify(result, ostr);
            return;
        }
    } catch (Poco::NotFoundException& e) {
        response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_BAD_REQUEST);
        Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
        root->set("status", static_cast<int>(response.getStatus()));
        root->set("detail", "request is incomplete");
        root->set("instance", uri.getPath());
        std::ostream& ostr = response.send();
        Poco::JSON::Stringifier::stringify(root, ostr);
    } catch (...) {
        response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_INTERNAL_SERVER_ERROR);
        Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
        root->set("status", static_cast<int>(response.getStatus()));
        root->set("detail", "internal error");
        root->set("instance", uri.getPath());
        std::ostream& ostr = response.send();
        Poco::JSON::Stringifier::stringify(root, ostr);
    }

    response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_NOT_FOUND);
    Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
    root->set("status", static_cast<int>(response.getStatus()));
    root->set("detail", "request not found");
    root->set("instance", uri.getPath());
    std::ostream& ostr = response.send();
    Poco::JSON::Stringifier::stringify(root, ostr);
}