#include <cpprest/http_client.h>
#include <cpprest/json.h>
#include "Data.h"
#include <iostream>
#include <vector>
#include <iostream>
#include <filesystem>

struct RequestInformation {
    std::string host;
    double timeout;
};

struct ApiKey {
    std::string key;
    std::string secret;
};

class Trader {
protected:
    ApiKey m_apiKey;
    RequestInformation m_requestInformation;
    unsigned int m_traderNumber;
    std::map<std::string, double> m_balances;
    std::map<std::string, Market> m_markets;
    Ticker m_ticker;
    std::vector<Trade> m_tradeHistory;
    std::map<std::string, std::map<std::string, std::map<std::string, GetOrder>>> m_orders;

    std::map<std::string, double> getBalances() const;
    Ticker getTicker(const std::string market) const;
    std::vector<Trade> getMarketTradeHistory(const std::string market) const;
    Balance getBalance(const std::string currency) const;
    GetOrder getOrder(const std::string uuid) const;
    BuyOrder submitBuyOrder(std::string market, double quantity, double price, double timeout) const;
    SellOrder submitSellOrder(std::string market, double quantity, double price, double timeout) const;
    bool submitCancelOrder(std::string uuid) const;
    std::map<std::string, std::map<std::string, std::map<std::string, GetOrder>>> getOrders() const;
    std::pair<std::string, std::string> getApiKey() const {
        // Grabs first line as key and second line as secret from a textfile - "apikey.txt" located relative to main.cpp
        auto apiKeyPath = std::filesystem::current_path() / "apikey.txt";
        std::ifstream file(apiKeyPath);
        std::string key;
        std::string value;
        std::getline(file, key);
        std::getline(file, value);
        file.close();
        return {key, value};
    };
    void setApiKey() {
        std::pair<std::string, std::string> currentApiKey {getApiKey()};
        m_apiKey.key = currentApiKey.first;
        m_apiKey.secret = currentApiKey.second;
    }

public:
    Trader(const unsigned int traderNumber, const std::string& host);
    double calculateBuyWorth(double currencyQuantity, double priceAt) const;
    double calculateSellWorth(double currencyQuantity, double priceAt) const;
    ~Trader(){};
    void downloadOrdersSpecificMarket(const std::string &market);
    OrderMarket getOrderBook(const std::string& market) const;
    std::map<std::string, Market> listMarkets() const;
};