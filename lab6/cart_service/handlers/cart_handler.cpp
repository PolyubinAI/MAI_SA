#include "cart_handler.h"

#include <Poco/Exception.h>
#include <Poco/JSON/Array.h>
#include <Poco/JSON/Object.h>
#include <Poco/Net/HTMLForm.h>
#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/URI.h>

#include <fstream>
#include <iostream>
#include <string>

#include "../../database/cart.h"
#include "../../server/auth.h"

void CartHandler::handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response) {
    Poco::Net::HTMLForm form(request, request.stream());
    Poco::URI uri(request.getURI());

    response.setChunkedTransferEncoding(true);
    response.setContentType("application/json");

    try {
        long user_id;
        std::string scheme, info;
        request.getCredentials(scheme, info);
        std::cout << "scheme: " << scheme << " identity: " << info << std::endl;

        if (scheme == "Bearer") {
            if (!JWT::extract(info, user_id)) {
                response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_FORBIDDEN);
                Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
                root->set("status", static_cast<int>(response.getStatus()));
                root->set("detail", "user not authorized");
                root->set("instance", uri.getPath());
                std::ostream& ostr = response.send();
                Poco::JSON::Stringifier::stringify(root, ostr);
                return;
            }
        } else {
            response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_FORBIDDEN);
            Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
            root->set("status", static_cast<int>(response.getStatus()));
            root->set("detail", "unsupported authorization scheme");
            root->set("instance", uri.getPath());
            std::ostream& ostr = response.send();
            Poco::JSON::Stringifier::stringify(root, ostr);
            return;
        }
        std::cout << "authorized user: " << user_id << std::endl;
        std::string id = std::to_string(user_id);

        if (uri.getPath() == "/cart") {
            if (request.getMethod() == Poco::Net::HTTPRequest::HTTP_GET) {
                std::optional<database::Cart> result = database::Cart::get_by_id(id);
                if (!result) {
                    database::Cart::create(id);
                }

                result = database::Cart::get_by_id(id);
                if (result) {
                    response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                    std::ostream& ostr = response.send();
                    Poco::JSON::Stringifier::stringify(result->expanded_items(), ostr);
                    return;
                }
                throw result;
            } else if (request.getMethod() == Poco::Net::HTTPRequest::HTTP_DELETE) {
                database::Cart::remove(id);
                response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                response.send();
                return;
            }
        } else if (uri.getPath() == "/cart/add" && request.getMethod() == Poco::Net::HTTPRequest::HTTP_PUT) {
            std::string item_id = form.get("item");

            std::optional<database::Cart> result = database::Cart::get_by_id(id);
            if (!result) {
                database::Cart::create(id);
                result = database::Cart::get_by_id(id);
            }

            if (result) {
                database::Cart& cart = result.value();
                cart.add_item(item_id);
                response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                std::ostream& ostr = response.send();
                Poco::JSON::Stringifier::stringify(cart.expanded_items(), ostr);
                return;
            }
            throw result;
        } else if (uri.getPath() == "/cart/remove" && request.getMethod() == Poco::Net::HTTPRequest::HTTP_PUT) {
            std::string item_id = form.get("item");

            std::optional<database::Cart> result = database::Cart::get_by_id(id);
            if (result) {
                database::Cart& cart = result.value();
                if (cart.remove_item(item_id)) {
                    response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                    std::ostream& ostr = response.send();
                    Poco::JSON::Stringifier::stringify(cart.expanded_items(), ostr);
                    return;
                }
            }

            response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_NOT_FOUND);
            Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
            root->set("status", static_cast<int>(response.getStatus()));
            root->set("detail", "item not in cart");
            root->set("instance", uri.getPath());
            std::ostream& ostr = response.send();
            Poco::JSON::Stringifier::stringify(root, ostr);
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