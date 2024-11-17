#pragma once

/**
 *  Contains declared endpoints and their info together with additional Swagger annotations.
 */

#include "dto.h"

#include "oatpp/web/server/api/ApiController.hpp"
#include "oatpp/macro/codegen.hpp"
#include "oatpp/macro/component.hpp"

#include OATPP_CODEGEN_BEGIN(ApiController) //<-- Begin Codegen

/**
 * Sample Api Controller.
 */
class controller : public oatpp::web::server::api::ApiController {
public:
  /**
   * Constructor with object mapper.
   * @param apiContentMappers - mappers used to serialize/deserialize DTOs.
   */
  controller(OATPP_COMPONENT(std::shared_ptr<oatpp::web::mime::ContentMappers>, apiContentMappers))
    : oatpp::web::server::api::ApiController(apiContentMappers)
  {}
public:
  
  ENDPOINT("GET", "/", root) {
    auto dto = dto::createShared();
    dto->statusCode = 200;
    dto->message = "Hello World!";
    return createDtoResponse(Status::CODE_200, dto);
  }
  
  ENDPOINT("GET", "/ctrl", root2) {
    auto dto = dto::createShared();
    dto->statusCode = 200;
    dto->message = "adam!";
    return createDtoResponse(Status::CODE_200, dto);
  }
  
};

#include OATPP_CODEGEN_END(ApiController) //<-- End Codegen
