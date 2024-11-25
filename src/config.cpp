#include "config.h"

#include <source_location>
#include <filesystem>
#include <fstream>
#include <variant>
#include <functional>
#include <utility>
#include <limits>
#include <algorithm> // std::find
#include <charconv>  // std::from_chars
#include <cstring>

#include "oatpp/core/base/Environment.hpp" //provide log

constexpr auto cfgName = "container_api.cfg";

constexpr auto fileWPathToCfg = "cfg_path.ini";

template <typename T>
constexpr bool validate_attribute(std::string_view value, T &attribute, const T low, const T high)
{
    // newSlave.id=std::stoi(value);
    if (std::from_chars(value.data(), value.data() + value.size(), attribute).ec != std::errc{} || attribute < low || attribute > high)
    {
        return false;
    }
    return true;
}

bool mik::config::read()
{
    constexpr std::pair<const char *, void (*)(std::string_view)> param[]
    {
        {"port", [](std::string_view val)
         {
            validate_attribute(val, _port, static_cast<decltype(_port)>(1), static_cast<decltype(_port)>(std::numeric_limits<decltype(_port)>::max()));
            OATPP_LOGI("port", "Server port from configuration : %d", _port);
         }},
        {"tls", [](std::string_view val)
         {
            _tls = val == "True" ? true : false;
            OATPP_LOGI("tls", "TLS is: %s", _tls ? "enabled" : "disalbed");
         }},
        {"channel", [](std::string_view val)
        {
            // 26 token#hash 7 2
            std::string_view channel, hash, bin_out, anl_out;
            std::size_t start_idx = 0, space_idx;
            uint8_t ch;
            uint16_t bin,anl;

            // search for separator index
            // 26[]token#hash 7 2
            space_idx = val.find(' ');
            // std::printf("start_idx=%d\n", start_idx);//0
            // std::printf("space_idx=%d\n", space_idx);//2
            
            // 26            
            channel = val.substr(start_idx, space_idx-start_idx);

            // string_view -> decltype(ch)
            validate_attribute(channel, ch, static_cast<decltype(ch)>(1), static_cast<decltype(ch)>(std::numeric_limits<decltype(ch)>::max()));

            //     token#hash
            start_idx=space_idx+1;
            // 26 token#hash[]7 2
            space_idx = val.find(' ', start_idx);
            // std::printf("start_idx=%d\n", start_idx);//3
            // std::printf("space_idx=%d\n", space_idx);//67
            hash = val.substr(start_idx, space_idx - start_idx);
            //                7
            start_idx = space_idx + 1;
            space_idx = val.find(' ', start_idx);
            // std::printf("start_idx=%d\n", start_idx);
            // std::printf("space_idx=%d\n", space_idx);
            bin_out = val.substr(start_idx, space_idx - start_idx);

            // string_view -> decltype(bin)
            validate_attribute(bin_out, bin, static_cast<decltype(bin)>(std::numeric_limits<decltype(bin)>::min()), static_cast<decltype(bin)>(std::numeric_limits<decltype(bin)>::max()));

            //                  2
            start_idx = space_idx + 1;
            // space_idx = val.find(' ', start_idx);
            // std::printf("start_idx=%d\n", start_idx);
            anl_out = val.substr(start_idx);

            // string_view -> decltype(anl)
            validate_attribute(anl_out, anl, static_cast<decltype(anl)>(std::numeric_limits<decltype(anl)>::min()), static_cast<decltype(anl)>(std::numeric_limits<decltype(anl)>::max()));

            auto container=_mapping.emplace_back(ch, mik::container{hash,bin,anl});

            //TODO:: logs do not work
            OATPP_LOGI("channel","New token hash:%s for channel:%d; bin:%d; anl:%d", container.second.hash.data(), container.first, container.second.bin_out, container.second.anl_out);
         }}

    };

    std::string cfgDirPath;

    if (!std::filesystem::exists(fileWPathToCfg))
    {
        OATPP_LOGE(std::source_location::current().file_name()+std::source_location::current().line(), "File with path to cfg directory:%s does not exist",fileWPathToCfg);
        return false;
    }
    std::ifstream file_ini(fileWPathToCfg);
            
    if (!file_ini.is_open())
    {
        OATPP_LOGE(std::source_location::current().file_name()+std::source_location::current().line(), "Configuration file:%s cannot be opened, check if you have sufficient permission\n",fileWPathToCfg);
        return false;
    }

    if (std::getline(file_ini,cfgDirPath))
    {
    }
    else
    {
        OATPP_LOGE(std::source_location::current().file_name()+std::source_location::current().line(), "File with path to cfg directory:%s does not contain content\n",fileWPathToCfg);
        return false;
    }

    /* concatenate cfg direction with file name /fs0/cfg/containers_api.cfg */
    std::filesystem::path cfgPath{std::filesystem::path(cfgDirPath) /= cfgName};

    if (!std::filesystem::exists(cfgPath))
    {
        OATPP_LOGE(std::source_location::current().file_name()+std::source_location::current().line(), "Configuration file:%s does not exist\n",cfgPath.c_str());
        return false;
    }    

    /* opens file */
    std::ifstream file(cfgPath);

    if (!file.is_open())
    {
        OATPP_LOGE(std::source_location::current().file_name()+std::source_location::current().line(), "Configuration file:%s cannot be opened\n",cfgPath);
        return false;
    }

    std::string line;

    /* read each line*/
    while (std::getline(file, line))
    {
        std::istringstream iss(line);
        std::string var, value;

        /* token 11 423hb4jhj234 */
        /* unpack paramname */
        // token
        iss >> var;

        /* unpack remaining data in line */
        //       11 423hb4jhj234

        // while (iss >> value);
        
        std::getline(iss >> std::ws, value);

        /* on wsl obtain \r on the end of line */
        if (!value.empty() && value.back() == '\r')
        {
            value.pop_back();
        }

            /* find recipe to given parametr name */
            auto it = std::find_if(std::begin(param), std::end(param), [&var](const auto &p)
                                   { return var == p.first; });
            if (it != std::end(param))
            {

                /* call recipe for parametr */
                it->second(value);
            }
        }

    return true;
}

int mik::config::match_channel_to_hash(unsigned char *hash,size_t hash_size)
{
    // OATPP_LOGE("adam"/* std::source_location::current().file_name() + std::source_location::current().line() */, "hash:%s size:%d", hash, hash_size);
    // OATPP_LOGE("adam"/* std::source_location::current().file_name() + std::source_location::current().line() */, "maphash:%s size:%d", _mapping[1].second.hash.data(), _mapping[0].second.hash.size());
    auto it = std::find_if(_mapping.begin(),_mapping.end(),[hash, hash_size](const auto& pair)
    {
        return 
        hash_size == pair.second.hash.size() 
        && 
        (std::memcmp(hash, pair.second.hash.data(), pair.second.hash.size()) == 0);
    });

    if (it != _mapping.end())
    {
        return it->first;
    }
    
    return -1;
}
