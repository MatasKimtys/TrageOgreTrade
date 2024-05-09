#include <cpprest/http_client.h>
#include <cpprest/json.h>
#include <uuid/uuid.h>
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
    
    void submitBuyOrder(const std::string& market, const double& quantity, const double& price, const double timeout);
    void submitSellOrder(const std::string& market, const double& quantity, const double& price, const double timeout);
    void submitCancelOrder(uuid_t UUID, const double timeout);
    std::map<std::string, double> getBalances() const;
    std::string getOrders();
    std::pair<std::string, std::string> getApiKey() const {
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
    ~Trader() {};
    std::tuple<std::string, bool, double, double> getBalance(const std::string& currency);
    void downloadOrdersSpecificMarket(const std::string &market);
    OrderMarket getSpecificMarketJson(const std::string& market) const;
    std::map<std::string, Market> listMarkets() const;
};