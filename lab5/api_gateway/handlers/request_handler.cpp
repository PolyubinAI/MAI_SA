#include "request_handler.h"

#include <Poco/Dynamic/Var.h>
#include <Poco/Exception.h>
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Parser.h>
#include <Poco/Net/HTMLForm.h>
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/Redis/Redis.h>
#include <Poco/StreamCopier.h>
#include <Poco/URI.h>

#include <fstream>
#include <iostream>
#include <string>

#include "../../database/cache.h"

using database::cache::Database;

std::string RequestHandler::getKey(const std::string& method, const std::string& path, const std::string& auth) {
    return method + ":" + path + ":" + auth;
}

std::pair<Poco::Net::HTTPResponse::HTTPStatus, std::string>
RequestHandler::sendRequest(const std::string& method, const std::string& base_path, const std::string& query, const std::string& basic_auth, const std::string& token) {
    Poco::URI uri(base_path + query);
    std::string path(uri.getPathAndQuery());
    if (path.empty()) {
        path = "/";
    }

    std::cout << "# api gateway: request " << base_path + query << std::endl;
    Poco::Net::HTTPClientSession session(uri.getHost(), uri.getPort());
    Poco::Net::HTTPRequest request(method, path, Poco::Net::HTTPMessage::HTTP_1_1);

    if (!basic_auth.empty()) {
        request.set("Authorization", "Basic " + basic_auth);
    } else if (!token.empty()) {
        request.set("Authorization", "Bearer " + token);
    }

    session.sendRequest(request);
    Poco::Net::HTTPResponse response;
    std::istream& rs = session.receiveResponse(response);

    std::stringstream ss;
    Poco::StreamCopier::copyStream(rs, ss);
    return {response.getStatus(), ss.str()};
}

void RequestHandler::handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response) {
    Poco::Net::HTMLForm form(request, request.stream());
    Poco::URI uri(request.getURI());

    response.setChunkedTransferEncoding(true);
    response.setContentType("application/json");

    try {
        std::string scheme, info, token;
        request.getCredentials(scheme, info);
        std::cout << "scheme: " << scheme << " identity: " << info << std::endl;
        
        if (request.getMethod() == Poco::Net::HTTPRequest::HTTP_GET) {
            Poco::Redis::BulkString cached_response = Database::get().get(getKey(request.getMethod(), uri.getPathAndQuery(), info));
            if (!cached_response.isNull()) {
                response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_OK);
                response.send() << cached_response.value();
                return;
            }
        }
        
        bool required_token = !((uri.getPath() == "/user" && request.getMethod() == Poco::Net::HTTPRequest::HTTP_POST) || uri.getPath().starts_with("/item"));
        
        if (required_token) {
            if (scheme == "Basic") {
                auto [status, content] = sendRequest(Poco::Net::HTTPRequest::HTTP_GET, _user_service, "/user/auth", info, "");
                if (status != Poco::Net::HTTPResponse::HTTPStatus::HTTP_OK) {
                    response.setStatus(status);
                    response.send() << content;
                    return;
                }

                Poco::JSON::Parser parser;
                Poco::Dynamic::Var result = parser.parse(content);
                Poco::JSON::Object::Ptr object = result.extract<Poco::JSON::Object::Ptr>();
                token = object->getValue<std::string>("token");
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
        }

        std::string base_url;
        if (uri.getPath().starts_with("/user")) {
            base_url = _user_service;
        } else if (uri.getPath().starts_with("/cart")) {
            base_url = _cart_service;
        } else if (uri.getPath().starts_with("/item")) {
            base_url = _item_service;
        }

        if (!base_url.empty()) {
            auto [status, content] = sendRequest(request.getMethod(), base_url, uri.getPathAndQuery(), "", token);

            if (request.getMethod() == Poco::Net::HTTPRequest::HTTP_GET && status == Poco::Net::HTTPResponse::HTTPStatus::HTTP_OK) {
                Database::get().set(getKey(request.getMethod(), uri.getPathAndQuery(), info), content, 60);
            }

            response.setStatus(status);
            response.send() << content;
            return;
        }
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