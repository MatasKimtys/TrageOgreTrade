#include <iostream>
#include <uuid/uuid.h>
#include <vector>

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
    std::string getOrdersSpecificMarket(const std::string &market);
};