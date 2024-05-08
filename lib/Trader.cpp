#include "Trader.h"
#include <tuple>
#include <fstream>

#include <cpprest/http_listener.h>              // HTTP server
#include <cpprest/json.h>                       // JSON library
#include <cpprest/uri.h>                        // URI library
#include <cpprest/ws_client.h>                  // WebSocket client
#include <cpprest/containerstream.h>            // Async streams backed by STL containers
#include <cpprest/interopstream.h>              // Bridges for integrating Async streams with STL and WinRT streams
#include <cpprest/rawptrstream.h>               // Async streams backed by raw pointer to memory
#include <cpprest/producerconsumerstream.h>     // Async streams for producer consumer scenarios
#include <cpprest/http_client.h>
#include <cpprest/filestream.h>

using namespace utility;                    // Common utilities like string conversions
using namespace web;                        // Common features like URIs.
using namespace web::http;                  // Common HTTP functionality
using namespace web::http::client;          // HTTP client features
using namespace concurrency::streams;       // Asynchronous streams

using namespace web::http::experimental::listener;          // HTTP server
using namespace web::experimental::web_sockets::client;     // WebSockets client
using namespace web::json;                                  // JSON library
    
Trader::Trader(const unsigned int traderNumber, const std::string& host) {
    m_requestInformation.host = host;
    m_traderNumber = traderNumber;
    setApiKey();
    std::cout << "Trader " << traderNumber << " created!\n";
}

void Trader::submitBuyOrder(const std::string& market, const double& quantity, const double& price, const double timeout)
{
    std::string path {"/order/buy"};
}

void Trader::submitSellOrder(const std::string& market, const double& quantity, const double& price, const double timeout)
{
    std::string path {"/order/sell"};
}

void Trader::submitCancelOrder(uuid_t UUID, const double timeout)
{
    std::string path {"/order/cancel"};

}

OrderMarket Trader::getSpecificMarketJson(const std::string& market) const {
    std::string url = m_requestInformation.host + "/orders/" + market;
    http_request request;
    request.set_method(methods::GET);
    http_client client(url);
    OrderMarket orderMarket;
    client.request(request).then([&orderMarket](http_response response) {
    try {
        if (response.status_code() == status_codes::OK) {
            std::cout << "Status code somethings right " << response.status_code() << std::endl;
            const auto& v = response.extract_string().get();
            web::json::value json = json::value::parse(v);


            orderMarket.status = json["success"].as_bool();
            for (auto [key, value] : json["buy"].as_object()) {
                orderMarket.buyOrders[std::stof(key)] = std::stof(value.as_string());
            }

            for (auto [key, value] : json["sell"].as_object()) {
                orderMarket.sellOrders[std::stof(key)] = std::stof(value.as_string());
            }

        } else {
            std::cerr << "Status code somethings wrong " << response.status_code() << std::endl;
        }
    } catch (const http_exception& e) {
        std::cout << "exception: " << e.error_code().message() << " " << e.what() << "\n";
    }
    }).wait();
    std::cout << std::endl;
        // "array:" << json.is_array() << "\n" <<
        // "bool:" << json.is_boolean() << "\n" <<
        // "int:" << json.is_integer() << "\n" <<
        // "double:" << json.is_double() << "\n" <<
        // "null:" << json.is_null() << "\n" <<
        // "string:" << json.is_string() << "\n" <<
        // "object:" << json.is_object() << "\n";
    return orderMarket;
}

void Trader::downloadOrdersSpecificMarket(const std::string& market)
{
    auto fileStream = std::make_shared<ostream>();
    pplx::task<web::json::value> jsonOutput;
    // Open stream to output file.
    pplx::task<void> requestTask = fstream::open_ostream(U(market + ".json")).then([=](ostream outFile)
    {
        *fileStream = outFile;

        // Create http_client to send the request.
        http_client client(U(m_requestInformation.host));

        // Build request URI and start the request.
        uri_builder builder(U("/orders/" + market));
        printf(builder.to_string().c_str(), std::string("\n").c_str());
        return client.request(methods::GET, builder.to_string());
    })

    // Handle response headers arriving.
    .then([=](http_response response)
    {
        printf("Received response status code:%u\n", response.status_code());

        // Write response body into the file.
        return response.body().read_to_end(fileStream->streambuf());
    })

    // Close the file stream.
    .then([=](size_t)
    {
        return fileStream->close();
    });

    // Wait for all the outstanding I/O to complete and handle any exceptions
    try
    {
        requestTask.wait();
    }
    catch (const std::exception &e)
    {
        printf("Error exception:%s\n", e.what());
    }
}

std::string Trader::getOrders()
{
    std::string path {"/account/orders"};
    return std::string();
}

std::tuple<std::string, bool, double, double> Trader::getBalance(const std::string& currency)
{
    std::string url {m_requestInformation.host + "/account/balance{" + currency + "}"};

    using Balance_t = std::tuple<std::string, bool, double, double>;

    Balance_t balance {"NaN", false, 0.0, 0.0};
    std::cout << "Get balance requested:" << 
        std::get<0>(balance) << "," << 
        std::get<1>(balance) << "," << 
        std::get<2>(balance) << "," << 
        std::get<3>(balance) << "\n";
    return balance;
}

std::string Trader::getBalances()
{
    std::string path {"/account/orders"};
    return std::string();
}
