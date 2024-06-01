#include "auth.h"

#include <Poco/Base64Decoder.h>
#include <Poco/JWT/Signer.h>
#include <Poco/JWT/Token.h>

#include <istream>
#include <ostream>
#include <string>
#include <utility>

#include "../config/config.h"

std::pair<std::string, std::string> get_credentials(const std::string identity) {
    std::istringstream istr(identity);
    std::ostringstream ostr;
    Poco::Base64Decoder b64in(istr);
    copy(std::istreambuf_iterator<char>(b64in),
         std::istreambuf_iterator<char>(),
         std::ostreambuf_iterator<char>(ostr));
    std::string decoded = ostr.str();

    size_t pos = decoded.find(':');
    std::string login = decoded.substr(0, pos);
    std::string password = decoded.substr(pos + 1);
    return {login, password};
}

namespace JWT {
std::string generate(long& id) {
    Poco::JWT::Token token;
    token.setType("JWT");
    token.setSubject("login");
    token.payload().set("id", id);
    token.setIssuedAt(Poco::Timestamp());

    Poco::JWT::Signer signer(Config::get().get_JWT_key());
    return signer.sign(token, Poco::JWT::Signer::ALGO_HS256);
}

bool extract(std::string& jwt_token, long& id) {
    if (jwt_token.length() == 0) {
        return false;
    }

    Poco::JWT::Signer signer(Config::get().get_JWT_key());
    try {
        Poco::JWT::Token token = signer.verify(jwt_token);
        if (token.payload().has("id")) {
            id = token.payload().getValue<long>("id");
            return true;
        }
        std::cout << "Not enough fields in token" << std::endl;

    } catch (...) {
        std::cout << "Token verification failed" << std::endl;
    }
    return false;
}
}  // namespace JWT