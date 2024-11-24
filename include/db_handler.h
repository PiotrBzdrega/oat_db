#pragma once

#include <cstdint>
#include <string_view>

namespace mik 
{
    struct db_handler
    {
        static int get_channel(std::string_view token);
    };
}