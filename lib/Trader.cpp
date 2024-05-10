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

#include "Trader.h"
#include <tuple>
#include <fstream>
#include <algorithm>

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
    m_balances = getBalances();
    m_markets = listMarkets();
    m_orders = getOrders();
    std::cout << "Trader " << traderNumber << " created!\n";
}

void Trader::submitBuyOrder(const std::string& market, const double& quantity, const double& price, const double timeout) {
    std::string path {"/order/buy"};
}

void Trader::submitSellOrder(const std::string& market, const double& quantity, const double& price, const double timeout) {
    std::string path {"/order/sell"};
}

void Trader::submitCancelOrder(std::string UUID, const double timeout) {
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
            std::cout << "Status code getSpecificMarketJson success " << response.status_code() << std::endl;
            const auto& v = response.extract_string().get();
            web::json::value json = json::value::parse(v);

            orderMarket.status = json["success"].as_bool();
            for (auto [key, value]:json["buy"].as_object()) {
                orderMarket.buyOrders[std::stof(key)] = std::stof(value.as_string());
            }

            for (auto [key, value]:json["sell"].as_object()) {
                orderMarket.sellOrders[std::stof(key)] = std::stof(value.as_string());
            }

        } else {
            std::cerr << "Status code getSpecificMarketJson failed " << response.status_code() << std::endl;
        }
    } catch (const http_exception& e) {
        std::cout << "exception: " << e.error_code().message() << " " << e.what() << "\n";
    }
    }).wait();
    return orderMarket;
}

std::map<std::string, Market> Trader::listMarkets() const {
    std::string url = m_requestInformation.host + "/markets";
    http_request request;
    request.set_method(methods::GET);
    http_client client(url);
    std::map<std::string, Market> markets;
    client.request(request).then([&markets](http_response response) {
    try {
        if (response.status_code() == status_codes::OK) {
            std::cout << "Status code listMarkets success " << response.status_code() << std::endl;
            const auto& v = response.extract_string().get();
            web::json::value json = json::value::parse(v);

            for (auto element : json.as_array()) {
                auto pair = element.as_object();
                for (auto [key, value] : pair) {
                    markets[key] = 
                        Market{
                            key, 
                            std::stof(value["initialprice"].as_string()),
                            std::stof(value["price"].as_string()),
                            std::stof(value["high"].as_string()),
                            std::stof(value["low"].as_string()),
                            std::stof(value["volume"].as_string()),
                            std::stof(value["bid"].as_string()),
                            std::stof(value["ask"].as_string())
                        };
                }
            }
        } else {
            std::cerr << "Status code listMarkets failed " << response.status_code() << std::endl;
        }
    } catch (const http_exception& e) {
        std::cout << "exception: " << e.error_code().message() << " " << e.what() << "\n";
    }
    }).wait();
    return markets;
}

void Trader::downloadOrdersSpecificMarket(const std::string& market) {
    auto fileStream = std::make_shared<ostream>();
    pplx::task<web::json::value> jsonOutput;
    // Open stream to output file.
    pplx::task<void> requestTask = fstream::open_ostream(U(market + ".json")).then([=](ostream outFile) {
        *fileStream = outFile;

        // Create http_client to send the request.
        http_client client(U(m_requestInformation.host));

        // Build request URI and start the request.
        uri_builder builder(U("/orders/" + market));
        printf(builder.to_string().c_str(), std::string("\n").c_str());
        return client.request(methods::GET, builder.to_string());
    })

    // Handle response headers arriving.
    .then([=](http_response response) {
        printf("Received response status code:%u\n", response.status_code());

        // Write response body into the file.
        return response.body().read_to_end(fileStream->streambuf());
    })

    // Close the file stream.
    .then([=](size_t) {
        return fileStream->close();
    });

    // Wait for all the outstanding I/O to complete and handle any exceptions
    try {
        requestTask.wait();
    } catch (const std::exception &e) {
        printf("Error exception:%s\n", e.what());
    }
}

std::map<std::string, std::map<std::string, std::map<std::string, GetOrder>>> Trader::getOrders() const {
    std::map<std::string, std::map<std::string, std::map<std::string, GetOrder>>> orders;
    std::string url = m_requestInformation.host + "/account/orders";
    http_request request;
    request.set_method(methods::POST);
    http_client_config config;
    credentials credentials(U(m_apiKey.key), U(m_apiKey.secret));
    config.set_credentials(credentials);
    http_client client(url, config);
    client.request(request)
    .then([&orders](http_response response) {
        try {
            if (response.status_code() == status_codes::OK) {
                std::cout << "Status code getOrders success " << response.status_code() << std::endl;
                const auto& v = response.extract_string().get();
                web::json::value json = json::value::parse(v);
                for (const auto& element : json.as_array()) {
                    auto pair = element.as_object();
                    const std::string type = pair["type"].as_string();
                    const std::string market = pair["market"].as_string();
                    const std::string uuid = pair["uuid"].as_string();
                    orders[market][type][uuid] = GetOrder(
                        uuid,
                        pair["date"].as_integer(),
                        type,
                        market,
                        std::stof(pair["price"].as_string()),
                        std::stof(pair["quantity"].as_string()),
                        std::stof(pair["fulfilled"].as_string())
                    );
                }
            } else {
                std::cerr << "Status code getOrders failed " << response.status_code() << std::endl;
            }
        } catch (const http_exception& e) {
            std::cout << "exception: " << e.error_code().message() << " " << e.what() << "\n";
        }
    }).wait();
    return orders;
}

                        // std::cout << 
                        //     value.is_array() << "\n" <<
                        //     value.is_boolean() << "\n" <<
                        //     value.is_double() << "\n" <<
                        //     value.is_integer() << "\n" <<
                        //     value.is_null() << "\n" <<
                        //     value.is_number() << "\n" <<
                        //     value.is_object() << "\n" <<
                        //     value.is_string() << "\n";

std::tuple<std::string, bool, double, double> Trader::getBalance(const std::string& currency) {
    std::string url {m_requestInformation.host + "/account/balance{" + currency + "}"};

    return {};
}

std::map<std::string, double> Trader::getBalances() const {
    std::map<std::string, double> balances;
    std::string url = m_requestInformation.host + "/account/balances";
    http_request request;
    request.set_method(methods::GET);
    http_client_config config;
    credentials credentials(U(m_apiKey.key), U(m_apiKey.secret));
    config.set_credentials(credentials);
    http_client client(url, config);
    client.request(request)
    .then([&balances](http_response response) {
        try {
            if (response.status_code() == status_codes::OK) {
                std::cout << "Status code getBalances success " << response.status_code() << std::endl;
                const auto& v = response.extract_string().get();
                web::json::value json = json::value::parse(v);
                for (auto [key, value] : json["balances"].as_object()) {
                    const double balance = std::stof(value.as_string());
                    if (balance < 1) {
                        continue;
                    }
                    balances[key] = balance;
                }
            } else {
                std::cerr << "Status code getBalances failed " << response.status_code() << std::endl;
            }
        } catch (const http_exception& e) {
            std::cout << "exception: " << e.error_code().message() << " " << e.what() << "\n";
        }
    }).wait();

    return balances;
}
