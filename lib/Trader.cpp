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
    m_ticker = getTicker("QUBIC-USDT");
    m_tradeHistory = getMarketTradeHistory("QUBIC-USDT");
    Balance qubicBalance = getBalance("QUBIC");
    // GetOrder order = getOrder("77e09670-ec62-4ac2-ae77-a27a31fc2cf9");
    // BuyOrder buyOrder = submitBuyOrder("QUBIC-USDT", 2, 0.00000528, 2.0);
    SellOrder sellOrder = submitSellOrder("QUBIC-USDT", qubicBalance.available, 0.00000534, 2.0);
    std::cout << "Trader " << traderNumber << " created!\n";
}

double Trader::calculateBuyWorth(double currencyQuantity, double priceAt) const {
    return currencyQuantity / priceAt;
}

double Trader::calculateSellWorth(double currencyQuantity, double priceAt) const {
    return currencyQuantity * priceAt;
}

OrderMarket Trader::getOrderBook(const std::string& market) const {
    std::string url = m_requestInformation.host + "/orders/" + market;
    http_request request;
    request.set_method(methods::GET);
    http_client client(url);
    OrderMarket orderMarket;
    client.request(request).then([&orderMarket](http_response response) {
    try {
        const auto& v = response.extract_string().get();
        web::json::value json = json::value::parse(v);

        orderMarket.status = json["success"].as_bool();
        for (auto [key, value]:json["buy"].as_object()) {
            orderMarket.buyOrders[std::stof(key)] = std::stof(value.as_string());
        }

        for (auto [key, value]:json["sell"].as_object()) {
            orderMarket.sellOrders[std::stof(key)] = std::stof(value.as_string());
        }
    } catch (const http_exception& e) {
        std::cerr << "getOrderBook exception: " << e.error_code().message() << " " << e.what() << "\n";
    } catch (...) {
        std::cerr << "Error in getOrderBook\n";
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
    } catch (const http_exception& e) {
        std::cerr << "listMarkets exception: " << e.error_code().message() << " " << e.what() << " status code:" << response.status_code() << "\n";
    } catch (...) {
        std::cerr << "Error in listMarkets\n";
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
    } catch (const http_exception &e) {
        std::cerr << "Exception: " << e.error_code().message() << " " << e.what() << " status code:" << e.error_code() << "\n";
    } catch (...) {
        std::cerr << "Error in downloadSpecificMarket\n";
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
        } catch (const http_exception& e) {
            std::cerr << "getOrders exception: " << e.error_code().message() << " " << e.what() << " status code:" << response.status_code() << "\n";
        } catch (...) {
        std::cerr << "Error in getOrders\n";
    }
    }).wait();
    return orders;
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
            const auto& v = response.extract_string().get();
            web::json::value json = json::value::parse(v);
            for (auto [key, value] : json["balances"].as_object()) {
                const double balance = std::stof(value.as_string());
                if (balance < 1) {
                    continue;
                }
                balances[key] = balance;
            }
        } catch (const http_exception& e) {
            std::cerr << "getBalances Exception: " << e.error_code().message() << " " << e.what() << " status code:" << response.status_code() << "\n";
        } catch (...) {
        std::cerr << "Error in getBalance\n";
    }
    }).wait();

    return balances;
}

Ticker Trader::getTicker(const std::string market) const {
    std::string url = m_requestInformation.host + "/ticker/" + market;
    http_request request;
    request.set_method(methods::GET);
    http_client client(url);
    Ticker ticker;
    client.request(request).then([&ticker](http_response response) {
    try {
        const auto& v = response.extract_string().get();
        web::json::value json = json::value::parse(v);
        std::map<std::string, std::string> tickerJsonData;
        for (auto [key, value] : json.as_object()) {
            if (!value.is_string()) {
                ticker.status = value.as_bool();
                continue;
            }
            tickerJsonData[key] = value.as_string();
        }
        ticker.ask = std::stof(tickerJsonData["ask"]);
        ticker.bid = std::stof(tickerJsonData["bid"]);
        ticker.high = std::stof(tickerJsonData["high"]);
        ticker.low = std::stof(tickerJsonData["low"]);
        ticker.initialPrice = std::stof(tickerJsonData["initialprice"]);
        ticker.price = std::stof(tickerJsonData["price"]);
        ticker.volume = std::stof(tickerJsonData["volume"]);
    } catch (const http_exception& e) {
        std::cerr << "getTicker exception: " << e.error_code().message() << " " << e.what() << " status code:" << response.status_code() << "\n";
    } catch (...) {
        std::cerr << "Error in getTicker\n";
    }
    }).wait();
    return ticker;
}

std::vector<Trade> Trader::getMarketTradeHistory(const std::string market) const {
    std::vector<Trade> tradeHistory;
    std::string url = m_requestInformation.host + "/history/" + market;
    http_request request;
    request.set_method(methods::GET);
    http_client client(url);
    client.request(request)
    .then([&tradeHistory](http_response response) {
        try {
            const auto& v = response.extract_string().get();
            web::json::value json = json::value::parse(v);
            for (const auto& element : json.as_array()) {
                auto object = element.as_object();
                const Trade trade = {
                    object["date"].as_integer(),
                    object["type"].as_string(),
                    std::stof(object["price"].as_string()),
                    std::stof(object["quantity"].as_string())
                };
                tradeHistory.emplace_back(trade);
            }
        } catch (const http_exception& e) {
            std::cerr << "getMarketTradeHistory exception: " << e.error_code().message() << " " << e.what() << " status code:" << response.status_code() << "\n";
        } catch (...) {
        std::cerr << "Error in getMarketTradeHistory\n";
    }
    }).wait();
    return tradeHistory;
}

Balance Trader::getBalance(const std::string currency) const {
    Balance balance;
    balance.currency = currency;
    std::string url = "/account/balance";
    http_request request;
    utility::string_t postData = U("currency=") + utility::conversions::to_string_t(currency);
    request.set_method(methods::POST);
    request.headers().set_content_type(U("application/x-www-form-urlencoded"));
    request.set_request_uri(U(url));
    request.set_body(postData);
    http_client_config config;
    credentials credentials(U(m_apiKey.key), U(m_apiKey.secret));
    config.set_credentials(credentials);
    http_client client(m_requestInformation.host, config);
    client.request(request)
    .then([&balance](http_response response) {
        try {
            if (response.status_code() != status_codes::OK) {
                std::cerr << "Bad client request\n";
            }
            const auto& v = response.extract_string().get();
            web::json::value json = json::value::parse(v);
            auto jsonObject = json.as_object();

            balance.available = std::stof(jsonObject["available"].as_string());
            balance.balance = std::stof(jsonObject["balance"].as_string());
            balance.requestStatus = jsonObject["success"].as_bool();
        } catch (const http_exception& e) {
            std::cerr << "getBalance Exception: " << e.error_code().message() << " " << e.what() << " status code:" << response.status_code() << "\n";
        }
    }).wait();

    return balance;
}

GetOrder Trader::getOrder(const std::string uuid) const {
    GetOrder order;
    std::string url = m_requestInformation.host + "/account/order/" + uuid;
    http_request request;
    request.set_method(methods::GET);
    http_client_config config;
    credentials credentials(U(m_apiKey.key), U(m_apiKey.secret));
    config.set_credentials(credentials);
    http_client client(url, config);
    client.request(request)
    .then([&order](http_response response) {
        try {
            if (response.status_code() != status_codes::OK) {
                std::cerr << "Bad client request\n";
            }
            const auto& v = response.extract_string().get();
            web::json::value json = json::value::parse(v);
            auto object = json.as_object();
            order.date = std::stoi(object["date"].as_string());
            order.fulfilled = std::stof(object["fulfilled"].as_string());
            order.market = object["market"].as_string();
            order.price = std::stof(object["price"].as_string());
            order.quantity = std::stof(object["quantity"].as_string());
            order.type = object["type"].as_string();
        } catch (const http_exception& e) {
            std::cerr << "getOrders exception: " << e.error_code().message() << " " << e.what() << " status code:" << response.status_code() << "\n";
        } catch (...) {
            std::cerr << "Error in getOrder\n";
        }
    }).wait();
    return order;
}

utility::string_t float_to_string(float value, int precision) {
    std::ostringstream out;
    out << std::fixed << std::setprecision(precision) << value;
    return conversions::to_string_t(out.str());
}

BuyOrder Trader::submitBuyOrder(std::string market, double quantity, double price, double timeout) const {
    BuyOrder buyOrder;
    std::string url = "/order/buy";
    http_request request;
    
    // Quantity should be used in a way that results in total spending resulting in more than 1 of e.g. USDT.
    utility::string_t quantity_str = float_to_string(quantity, 8);
    utility::string_t price_str = float_to_string(price, 8);
    utility::string_t postData = 
        U("market=") + market +
        U("&quantity=") + quantity_str +
        U("&price=") + price_str;
    request.set_method(methods::POST);
    request.headers().set_content_type(U("application/x-www-form-urlencoded"));
    request.set_request_uri(U(url));
    request.set_body(postData);
    http_client_config config;
    credentials credentials(U(m_apiKey.key), U(m_apiKey.secret));
    config.set_credentials(credentials);
    http_client client(m_requestInformation.host, config);
    client.request(request)
    .then([&buyOrder](http_response response) {
        try {
            if (response.status_code() != status_codes::OK) {
                std::cerr << "Bad client request\n";
            }
            const auto& v = response.extract_string().get();
            web::json::value json = json::value::parse(v);
            auto jsonObject = json.as_object();
            
            buyOrder.buyNewBalanceAvailable = std::stof(jsonObject["bnewbalavail"].as_string());
            buyOrder.sellNewBalanceAvailable = std::stof(jsonObject["snewbalavail"].as_string());
            buyOrder.quantity = std::stof(jsonObject["quantity"].as_string());
            buyOrder.status = jsonObject["success"].as_bool();
            buyOrder.uuid = jsonObject["uuid"].as_string();
        } catch (const http_exception& e) {
            std::cerr << "submitBuyOrder Exception: " << e.error_code().message() << " " << e.what() << " status code:" << response.status_code() << "\n";
        }
    }).wait();

    return buyOrder;
}

SellOrder Trader::submitSellOrder(std::string market, double quantity, double price, double timeout) const {
    SellOrder sellOrder;
    std::string url = "/order/sell";
    http_request request;
    
    // Quantity should be used in a way that results in total spending more than 1 of e.g. USDT.
    utility::string_t postData = 
        U("market=") + market +
        U("&quantity=") + float_to_string(quantity, 8) +
        U("&price=") + float_to_string(price, 8);
    request.set_method(methods::POST);
    request.headers().set_content_type(U("application/x-www-form-urlencoded"));
    request.set_request_uri(U(url));
    request.set_body(postData);
    http_client_config config;
    credentials credentials(U(m_apiKey.key), U(m_apiKey.secret));
    config.set_credentials(credentials);
    http_client client(m_requestInformation.host, config);
    client.request(request)
    .then([&sellOrder](http_response response) {
        try {
            if (response.status_code() != status_codes::OK) {
                std::cerr << "Bad client request\n";
            }
            const auto& v = response.extract_string().get();
            web::json::value json = json::value::parse(v);
            auto jsonObject = json.as_object();
            
            sellOrder.buyNewBalanceAvailable = std::stof(jsonObject["bnewbalavail"].as_string());
            sellOrder.sellNewBalanceAvailable = std::stof(jsonObject["snewbalavail"].as_string());
            sellOrder.quantity = std::stof(jsonObject["quantity"].as_string());
            sellOrder.status = jsonObject["success"].as_bool();
            sellOrder.uuid = jsonObject["uuid"].as_string();
        } catch (const http_exception& e) {
            std::cerr << "submitSellOrder Exception: " << e.error_code().message() << " " << e.what() << " status code:" << response.status_code() << "\n";
        }
    }).wait();

    return sellOrder;
}

bool Trader::submitCancelOrder(std::string uuid) const {
    std::string url = "/account/balance";
    bool success = false;
    http_request request;
    utility::string_t postData = U("uuid=") + utility::conversions::to_string_t(uuid);
    request.set_method(methods::POST);
    request.headers().set_content_type(U("application/x-www-form-urlencoded"));
    request.set_request_uri(U(url));
    request.set_body(postData);
    http_client_config config;
    credentials credentials(U(m_apiKey.key), U(m_apiKey.secret));
    config.set_credentials(credentials);
    http_client client(m_requestInformation.host, config);
    client.request(request)
    .then([&success](http_response response) {
        try {
            if (response.status_code() != status_codes::OK) {
                std::cerr << "Bad client request\n";
            }
            const auto& v = response.extract_string().get();
            web::json::value json = json::value::parse(v);
            auto jsonObject = json.as_object();
            success = jsonObject["success"].as_bool();
        } catch (const http_exception& e) {
            std::cerr << "getBalance Exception: " << e.error_code().message() << " " << e.what() << " status code:" << response.status_code() << "\n";
        }
    }).wait();

    return success;
}




    // std::cout << 
    //     "is_array:" << value.is_array() << "\n" <<
    //     "is_boolean:" << value.is_boolean() << "\n" <<
    //     "is_double:" << value.is_double() << "\n" <<
    //     "is_integer:" << value.is_integer() << "\n" <<
    //     "is_null:" << value.is_null() << "\n" <<
    //     "is_number:" << value.is_number() << "\n" <<
    //     "is_object:" << value.is_object() << "\n" <<
    //     "is_string:" << value.is_string() << "\n\n";