/** 
 *  This is an application’s entry point. Here Application Components are loaded, 
 *  Controller’s endpoints are added to the Router, and the server starts.
 */ 

#include <iostream>
#include "component.h"
#include "controller.h"

#include "config.h"

#include "oatpp/web/server/HttpConnectionHandler.hpp"

#include "oatpp/network/Server.hpp"
#include "oatpp/network/tcp/server/ConnectionProvider.hpp"

void run()
{

    OATPP_LOGI("MyApp", "Server running on port %d %d\n", mik::config::use_tls(), mik::config::get_port());

    /* Register Components in scope of run() method */
    component components(mik::config::use_tls(),mik::config::get_port());

    /* Get router component */
    OATPP_COMPONENT(std::shared_ptr<oatpp::web::server::HttpRouter>, router);

    /* Create MyController and add all of its endpoints to router */
    router->addController(std::make_shared<controller>());

    /* Get connection handler component */
    OATPP_COMPONENT(std::shared_ptr<oatpp::network::ConnectionHandler>, connectionHandler);

    /* TODO: create it dynamically */
    /* Get connection provider component */
    OATPP_COMPONENT(std::shared_ptr<oatpp::network::ServerConnectionProvider>, connectionProvider, mik::config::use_tls() ? "https" : "http");

    /* Create server which takes provided TCP connections and passes them to HTTP connection handler */
    oatpp::network::Server server(connectionProvider, connectionHandler);

    /* Print info about server port */
    OATPP_LOGI("MyApp", "Server running on port %s", connectionProvider->getProperty("port").getData());

    /* Run server */
    server.run();
}

int main() {

    /* Init oatpp Environment */
    oatpp::base::Environment::init();

    /* Read configuration file */
    mik::config::read();






    



    /* Run App */
    run();

    /* Destroy oatpp Environment */
    oatpp::base::Environment::destroy();

    return 0;

}


