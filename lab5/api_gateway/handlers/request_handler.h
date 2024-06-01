#pragma once

#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>

class RequestHandler : public Poco::Net::HTTPRequestHandler {
public:
    RequestHandler() {
        _user_service = std::getenv("USER_SERVICE");
        _item_service = std::getenv("ITEM_SERVICE");
        _cart_service = std::getenv("CART_SERVICE");
    }

    std::string getKey(const std::string& method, const std::string& path, const std::string& auth);

    void handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response);

    std::pair<Poco::Net::HTTPResponse::HTTPStatus, std::string>
    sendRequest(const std::string& method, const std::string& base_path, const std::string& query, const std::string& basic_auth, const std::string& token);

private:
    std::string _user_service;
    std::string _item_service;
    std::string _cart_service;
};