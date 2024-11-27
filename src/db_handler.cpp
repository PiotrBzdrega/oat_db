#include "db_handler.h"
#include "ssl.h"
#include "config.h"
// #include "dto.h" //Data Transfer Object

#include <cstring>

extern "C"
{
#include "bazainterface.h"
#include "bazaconvtime.h"
#include "dnpmacros.h"
}

#include <source_location>
#include "oatpp/core/base/Environment.hpp" //provide logs

enum
{
    DB_OK,
    DB_ERROR,
    DB_FIFO,
    DB_CONF,
    DB_OPEN,
    DB_AUTH,
    DB_HASH,
    DB_TIMEOUT,
    DB_CONTROL,
    DB_BINARY,
    DB_ANALOG,
    DB_LOG,
    DB_CFG,
    DB_VER,
    DB_RST,
    DB_EVENT,
} error;

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

std::optional<std::reference_wrapper<const mik::container>> mik::db_handler::get_container_ref(std::string_view token)
{
    constexpr auto sha_size = 64;
    uint8_t hash[sha_size + 1]{};
    sha(token.data(), hash);

    OATPP_LOGI(std::source_location::current().file_name() + std::source_location::current().line(), "token: %s gives hash: %s", token.data(), hash);
    return config::get_container_matching_hash(hash, sha_size);
}

bool mik::db_handler::read(char *cmd, const std::optional<std::reference_wrapper<const mik::container>> &container_opt, std::vector<bin> &bin_vec, std::vector<analog> &analog_vec)
{

    OATPP_LOGD("read", "command=%s\t chanel:%d\t bin:%d\t analog:%d\n", cmd, container_opt->get().channel, container_opt->get().bin_out, container_opt->get().anl_out);
    unsigned int err = 0;

    ParsedRange rangeTable[16];
    // char *command = FindToEnd(query, "=");

    long *a = static_cast<long *>(calloc(container_opt->get().anl_out > 0 ? container_opt->get().anl_out : 1, 4));
    unsigned char *f = static_cast<unsigned char *>(calloc(container_opt->get().anl_out > 0 ? container_opt->get().anl_out : 1, 1));
    unsigned char *b = static_cast<unsigned char *>(calloc(container_opt->get().bin_out > 0 ? container_opt->get().bin_out : 1, 1));

    // std::printf("command=%s\t %d\n", cmd, container_opt->get().anl_out);
    int rangeCnt = ParseRange(cmd, rangeTable, sizeof(rangeTable) / sizeof(ParsedRange));

    if (rangeCnt == 0)
    {
        if (baza_a(container_opt->get().channel, 0, a, f, container_opt->get().anl_out) < 0 /*!= CFG.a*/)
            err |= 1 << DB_ANALOG;
        if (baza_b(container_opt->get().channel, 0, b, container_opt->get().bin_out) < 0)
            err |= 1 << DB_BINARY;
    }
    else
    {
        int rngI = 0;
        ParsedRange *range = rangeTable;
        for (; rngI < rangeCnt; rngI++, range++)
        {
            if (range->isDigital)
            {
                size_t lenCor = range->start + range->count <= container_opt->get().bin_out ? range->count : container_opt->get().bin_out - range->start;
                // print("Reading bin from %d to %d\n", range->start, range->start + lenCor);
                if (baza_b(container_opt->get().channel, range->start, b + range->start, lenCor) < 0)
                    err |= 1 << DB_BINARY;
            }
            else
            {
                size_t lenCor = range->start + range->count <= container_opt->get().anl_out ? range->count : container_opt->get().anl_out - range->start;
                // print("Reading anl from %d to %d\n", range->start, range->start + lenCor);
                if (baza_a(container_opt->get().channel, range->start, a + range->start, f + range->start, lenCor) < 0)
                    err |= 1 << DB_ANALOG;
            }
        }
    }

    if (err)
    {
        free(a);
        free(f);
        free(b);
        return false;
    }
    else
    {
        if (rangeCnt)
        {
            int rngI = 0, idxI = 0;
            ParsedRange *range = rangeTable;
            for (rngI = 0, range = rangeTable; rngI < rangeCnt; rngI++, range++)
            {
                if (range->isDigital)
                {
                    /* BINARY */
                    size_t lenCor = range->start + range->count <= container_opt->get().bin_out ? range->count : container_opt->get().bin_out - range->start;
                    for (idxI = range->start; idxI < lenCor + range->start; idxI++)
                    {
                        bin_vec.push_back(bin{idxI, b[idxI]});
                    }
                }
                else
                {
                    /* ANALOG */
                    size_t lenCor = range->start + range->count <= container_opt->get().anl_out ? range->count : container_opt->get().anl_out - range->start;
                    for (idxI = range->start; idxI < lenCor + range->start; idxI++)
                    {
                        //TODO: verify if float/int is properly assigned
                        if (f[idxI] & 0x80)
                        {
                            analog_vec.push_back(analog{idxI, f[idxI], *(float *)&a[idxI]});
                        }
                        else
                        {
                            analog_vec.push_back(analog{idxI, f[idxI], *(int *)&a[idxI]});
                        }
                    }
                }
            }
        }
        else
        {
            /* BINARY */
            for (size_t i = 0; i < container_opt->get().bin_out; i++)
            {
                bin_vec.push_back(bin{i, b[i]});
            }

            /* ANALOG */
            for (size_t i = 0; i < container_opt->get().anl_out; i++)
            {
                // TODO: verify if float/int is properly assigned
                if (f[i] & 0x80)
                {
                    analog_vec.push_back(analog{i, f[i], *(float *)&a[i]});
                }
                else
                {
                    analog_vec.push_back(analog{i, f[i], *(int *)&a[i]});
                }
            }
        }
        
    }

    OATPP_LOGD("read", "gather data from database");
    for (const auto &b : bin_vec)
    {
        OATPP_LOGD("read", "binary[idx:%d, state:%02x]", b.index, b.state);
    }
    for (const auto &a : analog_vec)
    {
        if (std::holds_alternative<int>(a.value))
        {
            OATPP_LOGD("read", "analog[idx:%d, state:%02x, value:%d]", a.index, a.state, a.value);
        }
        else
        {
            OATPP_LOGD("read", "analog[idx:%d, state:%02x, value:%.3f]", a.index, a.state, a.value);
        }
    }

    free(a);
    free(f);
    free(b);

    return true;
}

bool mik::db_handler::read2(char *cmd, const std::optional<std::reference_wrapper<const mik::container>> &container_opt, oatpp::Object<read> &read_dto)
{

    OATPP_LOGD("read", "command=%s\t chanel:%d\t bin:%d\t analog:%d\n", cmd, container_opt->get().channel, container_opt->get().bin_out, container_opt->get().anl_out);
    unsigned int err = 0;

    ParsedRange rangeTable[16];
    // char *command = FindToEnd(query, "=");

    long *a = static_cast<long *>(calloc(container_opt->get().anl_out > 0 ? container_opt->get().anl_out : 1, 4));
    unsigned char *f = static_cast<unsigned char *>(calloc(container_opt->get().anl_out > 0 ? container_opt->get().anl_out : 1, 1));
    unsigned char *b = static_cast<unsigned char *>(calloc(container_opt->get().bin_out > 0 ? container_opt->get().bin_out : 1, 1));

    // std::printf("command=%s\t %d\n", cmd, container_opt->get().anl_out);
    int rangeCnt = ParseRange(cmd, rangeTable, sizeof(rangeTable) / sizeof(ParsedRange));

    if (rangeCnt == 0)
    {
        if (baza_a(container_opt->get().channel, 0, a, f, container_opt->get().anl_out) < 0 /*!= CFG.a*/)
            err |= 1 << DB_ANALOG;
        if (baza_b(container_opt->get().channel, 0, b, container_opt->get().bin_out) < 0)
            err |= 1 << DB_BINARY;
    }
    else
    {
        int rngI = 0;
        ParsedRange *range = rangeTable;
        for (; rngI < rangeCnt; rngI++, range++)
        {
            if (range->isDigital)
            {
                size_t lenCor = range->start + range->count <= container_opt->get().bin_out ? range->count : container_opt->get().bin_out - range->start;
                // print("Reading bin from %d to %d\n", range->start, range->start + lenCor);
                if (baza_b(container_opt->get().channel, range->start, b + range->start, lenCor) < 0)
                    err |= 1 << DB_BINARY;
            }
            else
            {
                size_t lenCor = range->start + range->count <= container_opt->get().anl_out ? range->count : container_opt->get().anl_out - range->start;
                // print("Reading anl from %d to %d\n", range->start, range->start + lenCor);
                if (baza_a(container_opt->get().channel, range->start, a + range->start, f + range->start, lenCor) < 0)
                    err |= 1 << DB_ANALOG;
            }
        }
    }

    if (err)
    {
        free(a);
        free(f);
        free(b);
        return false;
    }
    else
    {
        if (rangeCnt)
        {
            int rngI = 0, idxI = 0;
            ParsedRange *range = rangeTable;
            for (rngI = 0, range = rangeTable; rngI < rangeCnt; rngI++, range++)
            {
                if (range->isDigital)
                {
                    /* BINARY */
                    size_t lenCor = range->start + range->count <= container_opt->get().bin_out ? range->count : container_opt->get().bin_out - range->start;
                    for (idxI = range->start; idxI < lenCor + range->start; idxI++)
                    {
                        bin_vec.push_back(bin{idxI, b[idxI]});
                    }
                }
                else
                {
                    /* ANALOG */
                    size_t lenCor = range->start + range->count <= container_opt->get().anl_out ? range->count : container_opt->get().anl_out - range->start;
                    for (idxI = range->start; idxI < lenCor + range->start; idxI++)
                    {
                        // TODO: verify if float/int is properly assigned
                        if (f[idxI] & 0x80)
                        {
                            analog_vec.push_back(analog{idxI, f[idxI], *(float *)&a[idxI]});
                        }
                        else
                        {
                            analog_vec.push_back(analog{idxI, f[idxI], *(int *)&a[idxI]});
                        }
                    }
                }
            }
        }
        else
        {
            /* BINARY */
            for (size_t i = 0; i < container_opt->get().bin_out; i++)
            {
                bin_vec.push_back(bin{i, b[i]});
            }

            /* ANALOG */
            for (size_t i = 0; i < container_opt->get().anl_out; i++)
            {
                // TODO: verify if float/int is properly assigned
                if (f[i] & 0x80)
                {
                    analog_vec.push_back(analog{i, f[i], *(float *)&a[i]});
                }
                else
                {
                    analog_vec.push_back(analog{i, f[i], *(int *)&a[i]});
                }
            }
        }
    }

    OATPP_LOGD("read", "gather data from database");
    for (const auto &b : bin_vec)
    {
        OATPP_LOGD("read", "binary[idx:%d, state:%02x]", b.index, b.state);
    }
    for (const auto &a : analog_vec)
    {
        if (std::holds_alternative<int>(a.value))
        {
            OATPP_LOGD("read", "analog[idx:%d, state:%02x, value:%d]", a.index, a.state, a.value);
        }
        else
        {
            OATPP_LOGD("read", "analog[idx:%d, state:%02x, value:%.3f]", a.index, a.state, a.value);
        }
    }

    free(a);
    free(f);
    free(b);

    return true;
}

int mik::db_handler::baza_setBin(const char *payload)
{
    // 248 (StanyMASTER)
    char **paramsTable = NULL;
    // char** parseEndPtr = NULL;
    // char* paramsTablePtr = NULL;
    int paramsTablePos = 0;
    int rc = 0;
    char *parseEndPtr = NULL;
    StanyMASTER ustawStany;

    memset(&ustawStany, 0, sizeof(ustawStany));

    if (!payload)
        return -1;
    ustawStany.NumerKanalu = CFG.k;
    ustawStany.TypObiektu = 248;

    paramsTable = str_split(payload, ',');

    if (paramsTable[paramsTablePos])
    {
        int idx = -1;
        if (strtolError(paramsTable[paramsTablePos], &parseEndPtr, 10, &idx) || *parseEndPtr != 0)
        {
            rc = -1;
            goto returning;
        }
        ustawStany.Indeks = idx;
    }

    while (ustawStany.LiczbaObiektow < 2048 && paramsTable[++paramsTablePos])
    {
        int flg = -1;
        if (strtolError(paramsTable[paramsTablePos], &parseEndPtr, 0, &flg) || *parseEndPtr != 0)
        {
            rc = -1;
            goto returning;
        }
        ustawStany.pStany[ustawStany.LiczbaObiektow] = flg;
        std::printf("SETBIN index: %d, flag 0x%02X ", ustawStany.Indeks + ustawStany.LiczbaObiektow, ustawStany.pStany[ustawStany.LiczbaObiektow]);
        ustawStany.LiczbaObiektow++;

        /*print("StanyMASTER: \n\
        \tustawStan.NumerKanalu = %d;\n\
          \tustawStan.TypObiektu = %d;\n\
          \tustawStan.LiczbaObiektow = %d;\n\
        \tustawStan.Indeks = %d;\n\
        \tustawStan.Flaga = %d;\n\
        \tustawStan.CzasZmiany = %u\n",
        ustawStany.NumerKanalu, ustawStany.TypObiektu, ustawStany.LiczbaObiektow, ustawStany.Indeks, ustawStany.Flaga, *((unsigned int*)ustawStany.CzasZmiany));*/
        rc = baza_odczyt(&ustawStany) | (CFG.b < ustawStany.Indeks + ustawStany.LiczbaObiektow - 1);
        std::printf("Returned: %d\n", rc);
    }
returning:
    if (paramsTable)
    {
        for (paramsTablePos = 0; paramsTable[paramsTablePos] != NULL; paramsTablePos++)
            free(paramsTable[paramsTablePos]);
        free(paramsTable);
    }
    return rc;
}

int mik::db_handler::baza_setAnl(const char *payload)
{
    char **paramsTable = NULL;
    char *parseEndPtr = NULL;
    // char* paramsTablePtr = NULL;
    int paramsTablePos = 0;
    int rc = 0;
    float valueF = -1;
    TczasDNP tdnp;
    PomiaryMASTER ustawPomiar;

    getTdnp(tdnp);
    memset(&ustawPomiar, 0, sizeof(ustawPomiar));

    ustawPomiar.NumerKanalu = CFG.k;
    ustawPomiar.TypObiektu = 99;
    ustawPomiar.Wariacja = 0;
    ustawPomiar.Kwalifikator = 0;

    if (!payload)
        return -1;
    paramsTable = str_split(payload, ',');

    if (paramsTable[paramsTablePos])
    {
        int idx = -1;
        if (strtolError(paramsTable[paramsTablePos], &parseEndPtr, 10, &idx) || *parseEndPtr != 0)
        {
            rc = -1;
            goto returning;
        }
        ustawPomiar.Indeks = idx;
    }
    while (ustawPomiar.LiczbaObiektow < 100 && paramsTable[++paramsTablePos])
    {
        int flg = -1;
        if (strtolError(paramsTable[paramsTablePos], &parseEndPtr, 0, &flg) || *parseEndPtr != 0)
        {
            rc = -1;
            goto returning;
        }
        if (!paramsTable[++paramsTablePos])
        {
            rc = -1;
            goto returning;
        }
        ustawPomiar.pPomiary[ustawPomiar.LiczbaObiektow].Flaga = flg;

        if (ustawPomiar.pPomiary[ustawPomiar.LiczbaObiektow].Flaga & 0x80)
        {
            if (!strtofError(paramsTable[paramsTablePos], &parseEndPtr, &valueF) && *parseEndPtr == 0)
                ustawPomiar.pPomiary[ustawPomiar.LiczbaObiektow].Wartosc = *((k_ulong *)(&valueF));
            else
            {
                rc = -1;
                goto returning;
            }
        }
        else
        {
            int val = -1;
            if (strtolError(paramsTable[paramsTablePos], &parseEndPtr, 10, &val) || *parseEndPtr != 0)
            {
                rc = -1;
                goto returning;
            }
            ustawPomiar.pPomiary[ustawPomiar.LiczbaObiektow].Wartosc = val;
        }
        memcpy(ustawPomiar.pPomiary[ustawPomiar.LiczbaObiektow].Czas, tdnp, sizeof(tdnp));
        if (ustawPomiar.pPomiary[ustawPomiar.LiczbaObiektow].Flaga & 0x80)
        {
            print("SETANL index: %d, valuef: %f, flag 0x%02X \n", ustawPomiar.Indeks + ustawPomiar.LiczbaObiektow, valueF, ustawPomiar.pPomiary[ustawPomiar.LiczbaObiektow].Flaga);
        }
        else
        {
            print("SETANL index: %d, value: %d, flag 0x%02X \n", ustawPomiar.Indeks + ustawPomiar.LiczbaObiektow, (int)ustawPomiar.pPomiary[ustawPomiar.LiczbaObiektow].Wartosc, ustawPomiar.pPomiary[ustawPomiar.LiczbaObiektow].Flaga);
        }

        ustawPomiar.LiczbaObiektow++;
    }

    rc = baza_odczyt(&ustawPomiar) | (CFG.a < ustawPomiar.Indeks + ustawPomiar.LiczbaObiektow - 1);
returning:
    if (paramsTable)
    {
        for (paramsTablePos = 0; paramsTable[paramsTablePos] != NULL; paramsTablePos++)
            free(paramsTable[paramsTablePos]);
        free(paramsTable);
    }
    return rc;
}

int mik::db_handler::baza_c(int k, int i)
{
    Control_Data D;
    D.MasterNum = k;
    D.Type = 0;
    D.ControlIndex = i;
    D.ParamNum = 0;
    memset(D.Param, 0x00, 11);
    int rc = baza_weryfikacja(&D);
    /*if (rc)*/ std::printf("[%s] CTRL k:%d i:%d %d,%d rc=%d\n", __FUNCTION__, k, i, D.Param[0], D.Param[1], rc);
    if (rc)
        rc = 0xffff;
    else
        rc = D.Param[1] | (D.Param[0] << 8);
    return rc;
}

int mik::db_handler::baza_b(int chnlNo, int index, unsigned char *tableReadBin, int len)
{
    static int myMAX_StOdczyt = 2048;
    // TODO: MAX_StOdczyt 2048
    int ret = len;
    if (!tableReadBin || (len < 0) /*||(len>MAX_StOdczyt)*/)
        return -1;
    if (!len)
        return 0;
    StanyMASTER B;
    memset(&B, 0, sizeof(StanyMASTER));

    B.dl = 0;
    B.NumerKanalu = chnlNo;
    B.TypObiektu = 1;
    B.Wariacja = 0;
    B.Kwalifikator = 0;
    B.LiczbaObiektow = len;
    B.IND = 0;
    B.Indeks = index;

    while (len)
    {
        // print("[%s] indeks: %d, liczba: %d, pozostalo: %d, max: %d\n", __FUNCTION__, B.Indeks - index, B.LiczbaObiektow, len, myMAX_StOdczyt);
        if (len > myMAX_StOdczyt)
        {
            B.LiczbaObiektow = myMAX_StOdczyt;
            len -= myMAX_StOdczyt;
        }
        else
        {
            B.LiczbaObiektow = len;
            len = 0;
        }

        int rc = baza_odczyt(&B);
        if (rc)
        {
            std::printf("READBIN %s rc=%d\n", __FUNCTION__, rc);
            return -2;
        }
        if (B.LiczbaObiektow > 0)
            memcpy(tableReadBin + B.Indeks - index, B.pStany, B.LiczbaObiektow);
        B.Indeks += B.LiczbaObiektow;
    }
    return ret;
}

int mik::db_handler::baza_a(int chnlNo, int index, long *tableReadAnl, unsigned char *tableReadFlg, int len)
{
    static int myMAX_PomOdczyt = 100;
    int ret = len;
    if (!tableReadAnl || !tableReadFlg || (len < 0) /*||(l>MAX_PomOdczyt)*/)
        return -1;
    if (!len)
        return 0;
    PomiaryMASTER P;
    memset(&P, 0, sizeof(PomiaryMASTER));
    P.NumerKanalu = chnlNo;
    P.TypObiektu = 30;
    P.Indeks = index;
    while (len)
    {
        // print("[%s] indeks: %d, liczba: %d, pozostalo: %d, max: %d\n", __FUNCTION__, P.Indeks - index, P.LiczbaObiektow, len, myMAX_PomOdczyt);
        if (len > myMAX_PomOdczyt)
        {
            P.LiczbaObiektow = myMAX_PomOdczyt;
            len -= myMAX_PomOdczyt;
        }
        else
        {
            P.LiczbaObiektow = len;
            len = 0;
        }
        int rc = baza_odczyt(&P);
        if (rc)
        {
            std::printf("READANL %s rc=%d\n", __FUNCTION__, rc);
            return -2;
        }
        int i;
        for (i = 0; i < P.LiczbaObiektow; i++)
        {
            *tableReadAnl++ = P.pPomiary[i].Wartosc;
            *tableReadFlg++ = P.pPomiary[i].Flaga;
        }
        P.Indeks += P.LiczbaObiektow;
    }
    return ret;
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
