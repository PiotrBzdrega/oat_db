#pragma once

#include <cstdint> //uint8_t
#include <string>
#include <vector>

namespace mik
{
    class config
    {
    private:
        inline static std::vector<std::pair<uint8_t, std::string>> _mapping;
        inline static bool _tls{};
        inline static uint16_t _port{8000};

    public:
        static bool read(); // TODO: assure to be called only once
        static bool use_tls() { return _tls; }
        static uint16_t get_port() { return _port; }
        // config(/* args */);
        // ~config();
    };
}

