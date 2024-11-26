#pragma once

#include <cstdint> //uint8_t
#include <string>
#include <vector>
#include <optional>

namespace mik
{
    struct container
    {
        container(uint8_t channel, int bin_out, int anl_out) : channel(channel), bin_out(bin_out), anl_out(anl_out) {}
        // std::string hash;
        u_int8_t channel;
        int bin_out;
        int anl_out;
    };

    class config
    {
    private:
        // inline static std::vector<std::pair<uint8_t, std::string>> _mapping;
        // inline static std::vector<std::pair<u_int8_t, container>> _mapping;
        inline static std::vector<std::pair<std::string, container>> _mapping;
        inline static bool _tls{};
        inline static uint16_t _port{8000};

    public:
        static bool read(); // TODO: assure to be called only once
        static bool use_tls() { return _tls; }
        static uint16_t get_port() { return _port; }
        static int match_channel_to_hash(unsigned char *hash, size_t hash_size);
        // static std::optional<const std::pair<uint8_t, mik::container>&>get_entry_matching_hash(unsigned char *hash, size_t hash_size);
        static std::optional<std::reference_wrapper<const mik::container>> get_container_matching_hash(unsigned char *hash, size_t hash_size);
    };
}

