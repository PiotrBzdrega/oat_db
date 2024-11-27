#pragma once

/**
 *  Contains declared endpoints and their info together with additional Swagger annotations.
 */

#include "dto.h"
#include "auth_handler.h"
#include "db_handler.h"

#include "oatpp/web/server/api/ApiController.hpp"
#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/macro/component.hpp"
#include "oatpp/core/Types.hpp"

#include OATPP_CODEGEN_BEGIN(ApiController) //<-- Begin Codegen

/**
 * Sample Api Controller.
 */
class controller : public oatpp::web::server::api::ApiController
{
private:
    std::shared_ptr<AuthorizationHandler> m_authHandler = std::make_shared<MyBearerAuthorizationHandler>();
public:
    /**
     * Constructor with object mapper.
     * @param apiContentMappers - mappers used to serialize/deserialize DTOs.
     */
    controller(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper))
        : oatpp::web::server::api::ApiController(objectMapper)
    {
        // setDefaultAuthorizationHandler(std::make_shared<MyBearerAuthorizationHandler>("my-realm"));
    }

public:
    /* curl http://localhost:8000 */

    ENDPOINT("GET", "/", root)
    {
        mik::db_handler::get_channel("adamek");
        auto dto = dto::createShared();
        dto->statusCode = 200;
        dto->message = "Hello World!";
        return createDtoResponse(Status::CODE_200, dto);
    }

    /* 
        curl https://localhost:8000/read=sfjd -k \
        -H "Authorization: adamek" 
    */

    /*
        //wrong must be Bearer
        curl https://localhost:8000/read/=sfjd -k \
        -H "Authorization: adamek"
    */

    /*
        curl "https://localhost:8000/read/b0(10),a1(4)" -k  -H "Authorization: Bearer adamek"
    */

    ENDPOINT("GET", "/read/{content}", read,
             PATH(oatpp::String, content),
            //  HEADER(oatpp::String, token, "Authorization"),
             AUTHORIZATION(std::shared_ptr<BearerAuthorizationObject>, authObject, m_authHandler))
    {
        // OATPP_LOGD("db?read", "token='%s' content='%s' auth='%s'", token->c_str(), content->c_str(), authObject->token.get()->c_str());
        OATPP_LOGD("read", "token='%s' content='%s' ", authObject->token.get()->c_str(), content->c_str());
        // auto ch = mik::db_handler::get_channel(token->c_str());
        // OATPP_LOGD("db?read", "channel='%d'", ch);

        /* get container reference for matching token */
        // auto container_ref = mik::db_handler::get_container_ref(token->c_str());
        auto container_ref = mik::db_handler::get_container_ref(authObject->token.get()->c_str());
        OATPP_ASSERT_HTTP(container_ref, Status::CODE_401, "Unauthorized");

        std::vector<mik::bin> mik_bin;
        std::vector<mik::analog> mik_analog;

        char *query = strdup(content->c_str());
        mik::db_handler::read(query, container_ref, mik_bin, mik_analog);

        auto read_dto = read::createShared();

        mik::db_handler::read2(query, container_ref, read_dto);

        free(query);
        auto dto = dto::createShared();
        dto->statusCode = 200;
        dto->message = "Hello World!";
        return createDtoResponse(Status::CODE_200, dto);
    }

    // ENDPOINT("GET", "/set_bin/{content}", read,
    //          PATH(oatpp::String, content),
    //          //  HEADER(oatpp::String, token, "Authorization"),
    //          AUTHORIZATION(std::shared_ptr<BearerAuthorizationObject>, authObject, m_authHandler))
    // {
    //     // OATPP_LOGD("db?read", "token='%s' content='%s' auth='%s'", token->c_str(), content->c_str(), authObject->token.get()->c_str());
    //     OATPP_LOGD("read", "token='%s' content='%s' ", authObject->token.get()->c_str(), content->c_str());
    //     // auto ch = mik::db_handler::get_channel(token->c_str());
    //     // OATPP_LOGD("db?read", "channel='%d'", ch);

    //     /* get container reference for matching token */
    //     // auto container_ref = mik::db_handler::get_container_ref(token->c_str());
    //     auto container_ref = mik::db_handler::get_container_ref(authObject->token.get()->c_str());
    //     OATPP_ASSERT_HTTP(container_ref, Status::CODE_401, "Unauthorized");

    //     std::vector<mik::bin> mik_bin;
    //     std::vector<mik::analog> mik_analog;

    //     char *query = strdup(content->c_str());
    //     mik::db_handler::read(query, container_ref, mik_bin, mik_analog);
    //     free(query);
    //     auto dto = dto::createShared();
    //     dto->statusCode = 200;
    //     dto->message = "Hello World!";
    //     return createDtoResponse(Status::CODE_200, dto);
    // }

    /* curl -X POST http://localhost:8000/ctrl */

    ENDPOINT("POST", "/ctrl", control)
    {
        auto dto = dto::createShared();
        dto->statusCode = 200;
        dto->message = "adam!";
        return createDtoResponse(Status::CODE_200, dto);
    }

    /*
        curl -X POST https://localhost:8000/users -k \
         -H "User-Agent: MyTestClient/1.0" \
         -H "Authorization: Bearer 4e99e8c12de7e01535248d2bac85e732" \
         -H "Content-Type: text/plain" \
         -d "This is a test payload" \
         -k
     */

    /*
    curl -X POST https://localhost:8000/users -k \
     -H "User-Agent: MyTestClient/1.0" \
     -H "Authorization: Bearer 4e99e8c12de7e01535248d2bac85e733" \
     -H "Content-Type: text/plain" \
     -d "This is a test payload"
    */

    /*
    curl -X POST https://localhost:8000/users -k \
     -H "Authorization: Bearer 4e99e8c12de7e01535248d2bac85e732" \
     -d "This is a test payload"
    */

    ENDPOINT("POST", "/users", users,
             HEADER(oatpp::String, userAgent, "User-Agent"),
             HEADER(oatpp::String, userAuth, "Authorization"),
             HEADER(oatpp::String, contType, "Content-Type"),
             BODY_STRING(oatpp::String, userInfo),
             AUTHORIZATION(std::shared_ptr<BearerAuthorizationObject>, authObject, m_authHandler))
    {
        OATPP_ASSERT_HTTP(authObject->token == "4e99e8c12de7e01535248d2bac85e732", Status::CODE_401, "Unauthorized");
        OATPP_LOGD("Test", "header='%s\t%s\t%s' body='%s'", userAgent->c_str(), userAuth->c_str(), contType->c_str(), userInfo->c_str());
        return createResponse(Status::CODE_200, "OK");
    }

    // ENDPOINT("POST", "/token", process_token,
    //         BODY_DTO(dto::ObjectWrapper, requestDto))
    // {
    //   if (requestDto->token && requestDto->token == "asfsdg45t34ef45tge")
    //   {
    //     auto responseDto = dto::createShared();
    //     responseDto->statusCode = 200;
    //     responseDto->message = "Token is valid!";
    //     return createDtoResponse(Status::CODE_200, responseDto);
    //   }

    //   auto error_dto = dto::createShared();
    //   error_dto->statusCode = 400;
    //   error_dto->message = "Invalid Token";
    //   return createDtoResponse(Status::CODE_200, error_dto);
    // }
};

#include OATPP_CODEGEN_END(ApiController) //<-- End Codegen

#include "ssl.h"

