#pragma once

#include "Poco/DateTimeFormat.h"
#include "Poco/Net/HTTPServer.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/ServerSocket.h"
#include "Poco/Util/ServerApplication.h"

template <class HTTPRequestFactory>
class HTTPWebServer : public Poco::Util::ServerApplication {
public:
    int main([[maybe_unused]] const std::vector<std::string>& args) {
        Poco::Net::ServerSocket svs(Poco::Net::SocketAddress("0.0.0.0", 8080));
        Poco::Net::HTTPServer srv(new HTTPRequestFactory(Poco::DateTimeFormat::SORTABLE_FORMAT), svs, new Poco::Net::HTTPServerParams);
        srv.start();
        waitForTerminationRequest();
        srv.stop();

        return Poco::Util::Application::EXIT_OK;
    }
};
