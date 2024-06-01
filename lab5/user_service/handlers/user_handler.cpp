#include "user_handler.h"

#include <Poco/Exception.h>
#include <Poco/JSON/Object.h>
#include <Poco/Net/HTMLForm.h>
#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/URI.h>

#include <fstream>
#include <iostream>
#include <string>

#include "../../database/user.h"
#include "../../server/auth.h"

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
                        if (user.save()) {
                            response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                            std::ostream& ostr = response.send();
                            Poco::JSON::Stringifier::stringify(user.toJSON(), ostr);
                            return;
                        } else {
                            message = "login is already in use";
                        }
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
                long id;
                std::string scheme, info;
                request.getCredentials(scheme, info);
                std::cout << "scheme: " << scheme << " identity: " << info << std::endl;

                if (scheme == "Bearer") {
                    if (!JWT::extract(info, id)) {
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
                std::cout << "authorized user: " << id << std::endl;

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
                        if (user.update()) {
                            response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                            std::ostream& ostr = response.send();
                            Poco::JSON::Stringifier::stringify(user.toJSON(), ostr);
                            return;
                        } else {
                            message = "login is already in use";
                        }
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
                long id;
                std::string scheme, info;
                request.getCredentials(scheme, info);
                std::cout << "scheme: " << scheme << " identity: " << info << std::endl;

                if (scheme == "Bearer") {
                    if (!JWT::extract(info, id)) {
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
                std::cout << "authorized user: " << id << std::endl;

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
        } else if (uri.getPath() == "/user/search/name" && request.getMethod() == Poco::Net::HTTPRequest::HTTP_GET) {
            std::string fn = form.get("first_name");
            std::string ln = form.get("last_name");
            std::vector<database::User> users = database::User::search_by_name(fn, ln);
            Poco::JSON::Array arr;
            for (auto s : users) {
                arr.add(s.toJSON());
            }
            response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
            std::ostream& ostr = response.send();
            Poco::JSON::Stringifier::stringify(arr, ostr);
            return;
        } else if (uri.getPath() == "/user/search/login" && request.getMethod() == Poco::Net::HTTPRequest::HTTP_GET) {
            std::string login = form.get("login");
            std::vector<database::User> users = database::User::search_by_login(login);
            Poco::JSON::Array arr;
            for (auto s : users) {
                arr.add(s.toJSON());
            }
            response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
            std::ostream& ostr = response.send();
            Poco::JSON::Stringifier::stringify(arr, ostr);
            return;
        } else if (uri.getPath() == "/user/auth" && request.getMethod() == Poco::Net::HTTPRequest::HTTP_GET) {
            std::string scheme, info;
            request.getCredentials(scheme, info);
            std::cout << "scheme: " << scheme << " identity: " << info << std::endl;

            if (scheme == "Basic") {
                auto [login, password] = get_credentials(info);
                std::optional<long> id = database::User::auth(login, password);
                if (id) {
                    std::string token = JWT::generate(id.value());
                    response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                    Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
                    root->set("token", token);
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

            response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_UNAUTHORIZED);
            Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
            root->set("status", static_cast<int>(response.getStatus()));
            root->set("detail", "not authorized");
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