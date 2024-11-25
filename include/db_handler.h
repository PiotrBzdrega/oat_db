#pragma once

#include <cstdint>
#include <string_view>

namespace mik 
{
    typedef struct
    {
        size_t start;
        size_t count;
        bool isDigital;
    } ParsedRange;

    struct db_handler
    {
        static int get_channel(std::string_view token);
        static void read(char* cmd);
    private:
        static int findLastIndex(char* str, char x);
        static char * FindToEnd(char * R, char * znak1);
        static int strtolError (const char* str, char** endptr, int base, int* converted);
        static bool StringIsNumber(char *s);
        static char * FindBeetwen(char * R, char * znak1, char * znak2);
        static char** str_split(const char* a_str, const char a_delim);
        static int ParseRange(char *command, ParsedRange *rangeTable, int rangeTableSize);
    };
}