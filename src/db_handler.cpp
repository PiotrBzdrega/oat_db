#include "db_handler.h"

#include "ssl.h"
#include "config.h"

uint8_t mik::db_handler::get_channel(std::string_view token)
{
    uint8_t hash[128+1];
    sha(token.data(), hash);

    return 0;
}