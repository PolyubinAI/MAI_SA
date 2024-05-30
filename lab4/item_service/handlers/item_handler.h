#pragma once

#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"

class ItemHandler : public Poco::Net::HTTPRequestHandler {
public:
    ItemHandler(const std::string& format)
        : _format(format) {}

    void handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response);

private:
    std::string _format;
};