#pragma once

/** 
 *  It is a collection of components that will be loaded on application start. 
 *  Here we configure things like which ConnectionProvider to use, port to listen to,
 *  which ObjectMapper to use.
 */ 

#include "oatpp/web/server/HttpConnectionHandler.hpp"
#include "oatpp/web/mime/ContentMappers.hpp"

#include "oatpp/network/tcp/server/ConnectionProvider.hpp"
#include "oatpp/json/ObjectMapper.hpp"

#include "oatpp/macro/component.hpp"

/**
 *  Class which creates and holds Application components and registers components in oatpp::base::Environment
 *  Order of components initialization is from top to bottom
 */
class component {
public:
  
  /**
   *  Create ConnectionProvider component which listens on the port
   */
  OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::network::ServerConnectionProvider>, serverConnectionProvider)([] {
    return oatpp::network::tcp::server::ConnectionProvider::createShared({/* "0.0.0.0" */"localhost", 8000, oatpp::network::Address::IP_4});
  }());
  
  /**
   *  Create Router component
   */
  OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::web::server::HttpRouter>, httpRouter)([] {
    return oatpp::web::server::HttpRouter::createShared();
  }());
  
  /**
   *  Create ConnectionHandler component which uses Router component to route requests
   */
  OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::network::ConnectionHandler>, serverConnectionHandler)([] {
    OATPP_COMPONENT(std::shared_ptr<oatpp::web::server::HttpRouter>, router); // get Router component
    return oatpp::web::server::HttpConnectionHandler::createShared(router);
  }());
  
  /**
   *  Create ObjectMapper component to serialize/deserialize DTOs in Contoller's API
   */
  OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::web::mime::ContentMappers>, apiContentMappers)([] {

    auto json = std::make_shared<oatpp::json::ObjectMapper>();
    json->serializerConfig().json.useBeautifier = true;

    auto mappers = std::make_shared<oatpp::web::mime::ContentMappers>();
    mappers->putMapper(json);

    return mappers;

  }());

};