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

void mik::db_handler::read(char *cmd)
{
    ParsedRange rangeTable[16];
    // char *command = FindToEnd(query, "=");
    std::printf("command=%s\n",cmd);
    int rangeCnt = ParseRange(cmd, rangeTable, sizeof(rangeTable) / sizeof(ParsedRange));
    std::printf("command=%s\n", cmd);
}

int mik::db_handler::findLastIndex(char *str, char x)
{
    int index = -1;
    long i = 0;
    for (i = 0; i < strlen(str); i++)
        if (str[i] == x)
            index = i;
    return index;
}

char *mik::db_handler::FindToEnd(char *R, char *znak1)
{
    int pos1;
    int pos2;
    char *out;
    char *r = static_cast<char*>(calloc(strlen(R) + 1, sizeof(char)));

    memcpy(r, R, strlen(R));
    pos2 = findLastIndex(r, r[strlen(r) - 1]);
    pos1 = strcspn(r, znak1);

    if (pos1 >= 0 && pos2 >= 0 && pos2 > pos1)
    {
        out = static_cast<char*>(calloc(pos2 - pos1, sizeof(char)));
        r[pos2] = '\0';
        memcpy(out, &r[pos1 + 1], pos2 - pos1);
    }
    else
        out = NULL;

    free(r);
    return out;
}

int mik::db_handler::strtolError(const char *str, char **endptr, int base, int *converted)
{
    int convertedMem = -1;
    size_t zerosIter = 0;
    bool allZeros = true;
    if (!converted || !str)
        return -1;
    convertedMem = *converted;
    std::printf("[%s] str: %.15s..., before: %d,", __FUNCTION__, str, *converted);
    for (; zerosIter < strlen(str) && allZeros; zerosIter++)
    {
        if (isdigit(str[zerosIter]))
        {
            if (str[zerosIter] != '0')
                allZeros = false;
        }
        else
        {
            if (!zerosIter)
                allZeros = false;
            break;
        }
    }
    if (allZeros)
    {
        *converted = 0;
        if (endptr)
            *endptr = (char *)(str + zerosIter);
        std::printf(", after: %d (zero set), digits: %ul\n ", *converted, zerosIter);
        return 0;
    }
    else
    {
        *converted = strtol(str, endptr, base);
        if (*converted == 0)
        {
            *converted = convertedMem;
            std::printf(", after: %d (error in strtol)\n ", *converted);
            return -1;
        }
        std::printf("str: %s, return: %d, endPtr: 0x%02X\n ", str, *converted, (endptr ? **endptr : 0xff));
        return 0;
    }
}

bool mik::db_handler::StringIsNumber(char *s)
{
    bool out = true;
    long i = 0;

    for (i = 0; i < strlen(s); i++)
    {
        if (!(isdigit(s[i])))
            out = false;
    }

    return out;
}

char *mik::db_handler::FindBeetwen(char *R, char *znak1, char *znak2)
{
    int pos1;
    int pos2;
    char *out;
    char *r;
    r = static_cast<char *>(calloc(strlen(R) + 1, sizeof(char)));

    memcpy(r, R, strlen(R));
    pos2 = strcspn(r, znak2);
    pos1 = strcspn(r, znak1);

    if ((pos1 >= 0) && (pos2 >= 0) && (pos2 > pos1))
    {
        out = static_cast<char *>(calloc(pos2 - pos1, sizeof(char)));
        r[pos2] = '\0';
        memcpy(out, &r[pos1 + 1], pos2 - pos1);
    }
    else
        out = NULL;

    free(r);
    return out;
}

char **mik::db_handler::str_split(const char *a_str, const char a_delim)
{
    char **result = NULL;
    size_t count = 0;
    const char *last_comma = NULL;
    char delim[2] = {0, 0};
    char *a_str_tmp = (char *)calloc(1, strlen(a_str) + 1);
    const char *tmp_ptr = a_str_tmp;

    delim[0] = a_delim;
    memcpy(a_str_tmp, a_str, strlen(a_str));
    /* Count how many elements will be extracted. */
    while (*tmp_ptr)
    {
        if (a_delim == *tmp_ptr)
        {
            count++;
            last_comma = tmp_ptr;
        }
        tmp_ptr++;
    }
    /* Add space for trailing token. */

    if (a_str_tmp[0])
    {
        char *lastSignPtr = a_str_tmp + strlen(a_str_tmp) - 1;
        bool lastSignDeleted = false;
        count += last_comma < lastSignPtr;
        while (lastSignPtr > a_str_tmp && a_delim == *lastSignPtr)
        {
            if (count && lastSignDeleted)
                count--;
            *lastSignPtr = 0;
            lastSignPtr = a_str_tmp + strlen(a_str_tmp) - 1;
            lastSignDeleted = true;
        }
    }

    /* Add space for terminating null string so caller
       knows where the list of returned strings ends. */
    count++;
    result = (char **)malloc(sizeof(char *) * count);
    if (result)
    {
        size_t idx = 0;
        char *token = strtok(a_str_tmp, delim);

        if (!token || (!strlen(token) && !strlen(a_str_tmp)))
        {
            *(result + idx) = 0;
        }
        else
        {
            while (token)
            {
                if (idx >= count - 1)
                    break;
                *(result + idx++) = strdup(token);
                token = strtok(NULL, delim);
            }
            *(result + count - 1) = 0;
            if (idx <= count - 1)
            {
                *(result + idx) = 0;
            }
        }
    }
    free(a_str_tmp);
    return result;
}

int mik::db_handler::ParseRange(char *command, ParsedRange *rangeTable, int rangeTableSize)
{
    if (!command || !rangeTable)
        return -1;
    int i = 0, rc = 0, rangeTableIdx = 0;
    char **tokens = str_split(command, ',');
    ParsedRange element = {0, 0, false};
    if (tokens != NULL)
    {
        int t = 0;
        for (t = 0; *(tokens + t); t++)
        {
            char *zakres = *(tokens + t);
            if (strlen(zakres) > 0)
            {
                element.isDigital = (zakres[0] == 'b');
                if (element.isDigital || zakres[0] == 'a')
                {
                    char *binary = "b";
                    char *analog = "a";
                    char *start_with = FindBeetwen(zakres, (element.isDigital ? binary : analog), "(");
                    if (start_with != NULL)
                    {
                        if (StringIsNumber(start_with))
                        {
                            char *end;
                            int strt = -1;
                            if (strtolError(start_with, &end, 10, &strt) || *end != 0)
                            {
                                rc = -1;
                                free(start_with);
                                goto returning;
                            }
                            element.start = strt;
                            if (element.start >= 0)
                            {
                                char *num = FindBeetwen(zakres, "(", ")");
                                if (num != NULL)
                                {
                                    int cnt = -1;
                                    if (strtolError(num, &end, 10, &cnt) || *end != 0)
                                    {
                                        rc = -1;
                                        free(num);
                                        free(start_with);
                                        goto returning;
                                    }
                                    element.count = cnt;
                                    free(num);
                                }
                            }
                        }
                        free(start_with);
                    }
                    if (rangeTableIdx < rangeTableSize)
                        rangeTable[rangeTableIdx++] = element;
                }
                else
                    return -1;
            }
        }
    }
returning:
    if (tokens != NULL)
    {
        for (i = 0; tokens[i] != NULL; i++)
            free(tokens[i]);
        free(tokens);
    }
    std::printf("[%s] RangeTableSize = %d\n", __FUNCTION__, rangeTableIdx);
    /*while(rangeTableIdx) {
        rangeTableIdx--;
        print("RangeTable[%d]:\n\
        start = %d\n\
        count = %d\n\
        isDigital = %s\n",
        rangeTableIdx, rangeTable[rangeTableIdx].start, rangeTable[rangeTableIdx].count, rangeTable[rangeTableIdx].isDigital?"true":"false");
    }*/
    if (rangeTableIdx == 0)
        return rc;
    return rangeTableIdx;
}
