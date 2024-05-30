#pragma once

#include <string>
#include <utility>

std::pair<std::string, std::string> get_credentials(const std::string identity);

namespace JWT {
std::string generate(long& id);

bool extract(std::string& jwt_token, long& id);
}  // namespace JWT