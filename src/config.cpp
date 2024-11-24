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

#include "oatpp/core/base/Environment.hpp"

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
            OATPP_LOGI(std::source_location::current().file_name() + std::source_location::current().line(), "Server port from configuration : %d \n", _port);
         }},
        {"tls", [](std::string_view val)
         {
            _tls = val == "True" ? true : false;
            OATPP_LOGI(std::source_location::current().file_name() + std::source_location::current().line(), "TLS is: %s \n", _tls ? "enabled" : "disalbed");
         }},
        {"token", [](std::string_view val)
        {
            // 26  token#hash
            std::string_view channel_sv, hash;
            // search for separator index
            auto space_idx = val.find(' ');/////.
            // 26
            channel_sv = val.substr(0, space_idx);
            //     token#hash
            hash = val.substr(space_idx + 1, val.size() - 1);

            uint8_t channel;
            validate_attribute(channel_sv, channel, static_cast<decltype(channel)>(1), static_cast<decltype(channel)>(std::numeric_limits<decltype(channel)>::max()));

            _mapping.emplace_back(channel, hash);

            //TODO:: logs do not work
            OATPP_LOGI(std::source_location::current().file_name() + std::source_location::current().line(), "New token hash:%s for channel %d\n", hash.data(), channel);
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