#pragma once

/**
 * Describes the Data-Transfer-Object used in the “Hello World” response mentioned above.
 * In oatpp DTOs are used in ObjectMapping and Serialization/Deserialization.
 */

#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/data/mapping/type/Object.hpp"
#include "oatpp/core/Types.hpp"
#include "oatpp/encoding/Hex.hpp"

/* https://oatpp.io/docs/components/dto/#serialize-deserialize */
/* https://github.com/oatpp/oatpp-consul/blob/master/src/oatpp-consul/rest/DTOs.hpp */

#include OATPP_CODEGEN_BEGIN(DTO)

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class dto : public oatpp::DTO
{

    DTO_INIT(dto, DTO)

    DTO_FIELD(Int32, statusCode);
    DTO_FIELD(String, message);
};

class list_dto : public oatpp::DTO
{

    DTO_INIT(list_dto, DTO)

    DTO_FIELD(List<Object<dto>>, base);
    DTO_FIELD(String, info);
};

class vector_dto : public oatpp::DTO
{

    DTO_INIT(vector_dto, DTO)

    DTO_FIELD(Vector<Object<dto>>, base);
    DTO_FIELD(String, info);
};

class token_dto : public oatpp::DTO
{

    DTO_INIT(token_dto, DTO)

    DTO_FIELD(String, token);
};

class bin_dto : public oatpp::DTO
{
    /* binary:[{index:0,state:0x80},{index:1,state:0x81}] */
    DTO_INIT(bin_dto, DTO)

    DTO_FIELD(Int16, index);

    DTO_FIELD(Int8, state); // TODO: must be print in hex 02X
};

class analog_dto : public oatpp::DTO
{

    DTO_INIT(analog_dto, DTO)

    DTO_FIELD(Int16, index);

    DTO_FIELD(Int8, state); // TODO: must be print in hex 02X

    DTO_FIELD(Float32, value); // TODO: must be print in hex 02X
};

class read_dto : public oatpp::DTO
{

    DTO_INIT(read_dto, DTO)

    DTO_FIELD(Fields<List<Object<bin_dto>>>, bins);
    DTO_FIELD(Fields<List<Object<analog_dto>>>, analogs);

    DTO_FIELD(String, info);
};



#include OATPP_CODEGEN_END(DTO)
