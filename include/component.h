#pragma once

/**
 *  It is a collection of components that will be loaded on application start.
 *  Here we configure things like which ConnectionProvider to use, port to listen to,
 *  which ObjectMapper to use.
 */

#include "oatpp/web/server/HttpConnectionHandler.hpp"
// #include "oatpp/web/mime/ContentMappers.hpp"

#include "oatpp/network/tcp/server/ConnectionProvider.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"

#include "oatpp/core/macro/component.hpp"

#include "oatpp-openssl/server/ConnectionProvider.hpp"
#include "oatpp-openssl/Config.hpp"

/**
 *  Class which creates and holds Application components and registers components in oatpp::base::Environment
 *  Order of components initialization is from top to bottom
 */
class component
{
private:
    bool _use_tls;
    uint16_t _port;

public:
    component(bool use_tls, uint16_t port) : _use_tls(use_tls), _port(port) {}

    OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::network::ServerConnectionProvider>, serverSecureConnectionProvider)
    ("https",[this]
    {
        OATPP_LOGD("oatpp::openssl::Config", "pem='%s'", CERT_PEM_PATH);
        OATPP_LOGD("oatpp::openssl::Config", "crt='%s'", CERT_CRT_PATH);
        auto config = oatpp::openssl::Config::createDefaultServerConfigShared(CERT_CRT_PATH, CERT_PEM_PATH /* private key */);
        return oatpp::openssl::server::ConnectionProvider::createShared(config, {/* "0.0.0.0" */ "localhost", _port, oatpp::network::Address::IP_4});
     }());

    /* TODO: when both components are created then both ports are binded, if ports are same it cannot bind to same port */
    // OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::network::ServerConnectionProvider>, serverConnectionProvider)
    // ("http", [this]
    //  { return oatpp::network::tcp::server::ConnectionProvider::createShared({"localhost", _port, oatpp::network::Address::IP_4}); }());

    /**
     *  Create Router component
     */
    OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::web::server::HttpRouter>, httpRouter)
    ([]
     { return oatpp::web::server::HttpRouter::createShared(); }());

    /**
     *  Create ConnectionHandler component which uses Router component to route requests
     */
    OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::network::ConnectionHandler>, serverConnectionHandler)
    ([]
     {
    OATPP_COMPONENT(std::shared_ptr<oatpp::web::server::HttpRouter>, router); // get Router component
    return oatpp::web::server::HttpConnectionHandler::createShared(router); }());

    /**
     *  Create ObjectMapper component to serialize/deserialize DTOs in Contoller's API
     */
    OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::data::mapping::ObjectMapper>, apiObjectMapper)
    ([]
     { return oatpp::parser::json::mapping::ObjectMapper::createShared(); }());
};