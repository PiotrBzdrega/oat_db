#pragma once
#include <cstdint> //uint32_t
#include <string> 

namespace containers
{
    class config
    {
    private:
        inline static uint32_t _channel{};
        std::string _hash_from_token{};
    public:
        static bool read(std::string_view arguments);
        // config(/* args */);
        // ~config();
    };
}

