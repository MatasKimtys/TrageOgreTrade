#include <cpprest/http_client.h>
#include <cpprest/json.h>
#include <uuid/uuid.h>
#include "Data.h"
#include <iostream>
#include <vector>
#include <iostream>

struct RequestInformation {
    std::string host;
    double timeout;
};

class Trader {
private:
    RequestInformation m_requestInformation;
    unsigned int m_traderNumber;
    void submitBuyOrder(const std::string& market, const double& quantity, const double& price, const double timeout);
    void submitSellOrder(const std::string& market, const double& quantity, const double& price, const double timeout);
    void submitCancelOrder(uuid_t UUID, const double timeout);
    std::string getOrders();
    std::string getBalances();

public:
    Trader(const unsigned int traderNumber, const std::string& host);
    ~Trader() {};
    std::tuple<std::string, bool, double, double> getBalance(const std::string& currency);
    void downloadOrdersSpecificMarket(const std::string &market);
    OrderMarket getSpecificMarketJson(const std::string& market) const;
};