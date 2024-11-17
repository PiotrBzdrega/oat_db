#pragma once

/**
 * Describes the Data-Transfer-Object used in the “Hello World” response mentioned above. 
 * In oatpp DTOs are used in ObjectMapping and Serialization/Deserialization.
 */ 

#include "oatpp/macro/codegen.hpp"
#include "oatpp/Types.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class dto : public oatpp::DTO {
  
  DTO_INIT(dto, DTO)
  
  DTO_FIELD(Int32, statusCode);
  DTO_FIELD(String, message);
  
};

#include OATPP_CODEGEN_END(DTO)
