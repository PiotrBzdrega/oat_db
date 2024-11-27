#pragma once

#include <cstdint>
#include <string_view>
#include <optional>
#include <variant>
#include "config.h"
#include "dto.h" //Data Transfer Object

namespace mik 
{
    struct bin
    {
        int index;
        unsigned char state;
    };

    struct analog
    {
        int index;
        unsigned char state;
        std::variant<int, float> value; // Holds either int or float
    };

    typedef struct
    {
        size_t start;
        size_t count;
        bool isDigital;
    } ParsedRange;

    struct db_handler
    {
        static int get_channel(std::string_view token);
        static std::optional<std::reference_wrapper<const mik::container>> get_container_ref(std::string_view token);
        static bool read(char *cmd, const std::optional<std::reference_wrapper<const mik::container>> &container_opt, std::vector<bin> &bin_vect, std::vector<analog> &analog_vect);
        static bool read2(char *cmd, const std::optional<std::reference_wrapper<const mik::container>> &container_opt, oatpp::Object<read_dto> &dto);
        static bool ctrl_bin(char *cmd, const std::optional<std::reference_wrapper<const mik::container>> &container_opt);
        static bool ctrl_analog(char *cmd, const std::optional<std::reference_wrapper<const mik::container>> &container_opt);
        static bool set_bin(char *cmd, const std::optional<std::reference_wrapper<const mik::container>> &container_opt);
        static bool set_analog(char *cmd, const std::optional<std::reference_wrapper<const mik::container>> &container_opt);

    private:
        static int baza_setBin(const char* payload);
        static int baza_setAnl(const char* payload);
        static int baza_c(int k, int i);
        static int baza_b(int chnlNo, int index, unsigned char * tableReadBin, int len);
        static int baza_a(int chnlNo, int index, long *tableReadAnl, unsigned char *tableReadFlg, int len);
        static int findLastIndex(char *str, char x);
        static char * FindToEnd(char * R, char * znak1);
        static int strtolError (const char* str, char** endptr, int base, int* converted);
        static bool StringIsNumber(char *s);
        static char * FindBeetwen(char * R, char * znak1, char * znak2);
        static char** str_split(const char* a_str, const char a_delim);
        static int ParseRange(char *command, ParsedRange *rangeTable, int rangeTableSize);
    };
}