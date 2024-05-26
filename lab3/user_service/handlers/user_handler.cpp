#include "user_handler.h"

#include <Poco/Exception.h>
#include <Poco/Net/HTMLForm.h>
#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/URI.h>

#include <fstream>
#include <iostream>
#include <string>

#include "../../database/user.h"

void UserHandler::handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response) {
    Poco::Net::HTMLForm form(request, request.stream());
    Poco::URI uri(request.getURI());

    response.setChunkedTransferEncoding(true);
    response.setContentType("application/json");

    try {
        if (uri.getPath() == "/user") {
            if (request.getMethod() == Poco::Net::HTTPRequest::HTTP_GET) {
                long id = std::stol(form.get("id"));

                std::optional<database::User> result = database::User::get_by_id(id);
                if (result) {
                    response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                    std::ostream& ostr = response.send();
                    Poco::JSON::Stringifier::stringify(result->toJSON(), ostr);
                    return;
                } else {
                    response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_NOT_FOUND);
                    Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
                    root->set("status", static_cast<int>(response.getStatus()));
                    root->set("detail", "user not found");
                    root->set("instance", uri.getPath());
                    std::ostream& ostr = response.send();
                    Poco::JSON::Stringifier::stringify(root, ostr);
                    return;
                }
            } else if (request.getMethod() == Poco::Net::HTTPRequest::HTTP_POST) {
                std::string message;
                if (form.has("first_name") && form.has("last_name") && form.has("email") && form.has("phone") && form.has("login") && form.has("password")) {
                    database::User user;
                    user.first_name() = form.get("first_name");
                    user.last_name() = form.get("last_name");
                    user.email() = form.get("email");
                    user.phone() = form.get("phone");
                    user.login() = form.get("login");
                    user.password() = form.get("password");
                    user.hash_password();

                    bool valid_request = true;
                    std::string reason;

                    if (!user.check_name(reason)) {
                        valid_request = false;
                        message += reason;
                        message += "\n";
                    }

                    if (!user.check_email(reason)) {
                        valid_request = false;
                        message += reason;
                        message += "\n";
                    }

                    if (valid_request) {
                        user.save();
                        response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                        std::ostream& ostr = response.send();
                        Poco::JSON::Stringifier::stringify(user.toJSON(), ostr);
                        return;
                    }
                }

                if (message.empty()) {
                    message = "user information is incomplete";
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
                long id = std::stol(form.get("id"));

                std::optional<database::User> result = database::User::get_by_id(id);
                if (result) {
                    database::User& user = result.value();
                    user.first_name() = form.get("first_name", user.first_name());
                    user.last_name() = form.get("last_name", user.last_name());
                    user.email() = form.get("email", user.email());
                    user.phone() = form.get("phone", user.phone());
                    user.login() = form.get("login", user.login());
                    if (form.has("password")) {
                        user.password() = form.get("password");
                        user.hash_password();
                    }

                    bool valid_request = true;
                    std::string message, reason;

                    if (!user.check_name(reason)) {
                        valid_request = false;
                        message += reason;
                        message += "\n";
                    }

                    if (!user.check_email(reason)) {
                        valid_request = false;
                        message += reason;
                        message += "\n";
                    }

                    if (valid_request) {
                        user.update();
                        response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                        std::ostream& ostr = response.send();
                        Poco::JSON::Stringifier::stringify(user.toJSON(), ostr);
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
                root->set("detail", "user not found");
                root->set("instance", uri.getPath());
                std::ostream& ostr = response.send();
                Poco::JSON::Stringifier::stringify(root, ostr);
                return;
            } else if (request.getMethod() == Poco::Net::HTTPRequest::HTTP_DELETE) {
                long id = std::stol(form.get("id"));

                if (database::User::remove(id)) {
                    response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                    response.send();
                    return;
                } else {
                    response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_NOT_FOUND);
                    Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
                    root->set("status", static_cast<int>(response.getStatus()));
                    root->set("detail", "user not found");
                    root->set("instance", uri.getPath());
                    std::ostream& ostr = response.send();
                    Poco::JSON::Stringifier::stringify(root, ostr);
                    return;
                }
            }
        } else if (uri.getPath() == "/user/search" && request.getMethod() == Poco::Net::HTTPRequest::HTTP_GET) {
            std::string fn = form.get("first_name");
            std::string ln = form.get("last_name");
            auto results = database::User::search_by_name(fn, ln);
            Poco::JSON::Array arr;
            for (auto s : results) {
                arr.add(s.toJSON());
            }
            response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
            std::ostream& ostr = response.send();
            Poco::JSON::Stringifier::stringify(arr, ostr);
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