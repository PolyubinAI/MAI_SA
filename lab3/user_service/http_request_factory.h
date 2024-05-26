#pragma once

#include <iostream>

#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPRequestHandlerFactory.h"
#include "Poco/Net/HTTPServerRequest.h"

#include "handlers/user_handler.h"

class HTTPRequestFactory : public Poco::Net::HTTPRequestHandlerFactory {
public:
    HTTPRequestFactory(const std::string& format)
        : _format(format) {}

    Poco::Net::HTTPRequestHandler* createRequestHandler(const Poco::Net::HTTPServerRequest& request) {
        std::cerr << "request:" << request.getURI() << std::endl;
        if (request.getURI().starts_with("/user")) {
            return new UserHandler(_format);
        }
        return 0;
    }

private:
    std::string _format;
};
