#include "db_handler.h"
#include "ssl.h"
#include "config.h"
#include <cstring>

#include <source_location>
#include "oatpp/core/base/Environment.hpp" //provide logs

int mik::db_handler::get_channel(std::string_view token)
{
    constexpr auto sha_size=64;
    uint8_t hash[sha_size + 1]{};
    sha(token.data(), hash);
    OATPP_LOGI(std::source_location::current().file_name() + std::source_location::current().line(), "token: %s gives hash: %s", token.data(), hash);

    auto ch = config::match_channel_to_hash(hash, sha_size);

    OATPP_LOGI(std::source_location::current().file_name() + std::source_location::current().line(), "hash%sfound in configuration file; %d ", ch == -1 ? " not " : " ", ch);
    return ch;
}