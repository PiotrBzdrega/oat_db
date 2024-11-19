#pragma once

#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
// #include "oatpp/utils/Conversion.hpp"
#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core//macro/component.hpp"


    class BearerAuthorizationObject : public oatpp::web::server::handler::AuthorizationObject {
    public:
      oatpp::String user;
      oatpp::String password;
      oatpp::String token;
    };

    class MyBearerAuthorizationHandler : public oatpp::web::server::handler::BearerAuthorizationHandler {
    public:

      MyBearerAuthorizationHandler()
        : oatpp::web::server::handler::BearerAuthorizationHandler("custom-bearer-realm")
      {}

      std::shared_ptr<AuthorizationObject> authorize(const oatpp::String& token) override {

        if(token == "4e99e8c12de7e01535248d2bac85e732") {
          auto obj = std::make_shared<BearerAuthorizationObject>();
          obj->user = "foo";
          obj->password = "bar";
          obj->token = token;
          return obj;
        }

        return nullptr;
      }

    };