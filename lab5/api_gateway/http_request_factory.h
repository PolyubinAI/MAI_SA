#pragma once

#include <iostream>

#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPRequestHandlerFactory.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "handlers/request_handler.h"

class HTTPRequestFactory : public Poco::Net::HTTPRequestHandlerFactory {
public:
    HTTPRequestFactory() = default;

    Poco::Net::HTTPRequestHandler* createRequestHandler(const Poco::Net::HTTPServerRequest& request) {
        std::cerr << "request:" << request.getMethod() << ":" << request.getURI() << std::endl;
        return new RequestHandler();
    }
};
