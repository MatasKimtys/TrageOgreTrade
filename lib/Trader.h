#include <iostream>
#include <uuid/uuid.h>

struct RequestInformation {
    std::string host;
    std::string path;
    double timeout;
};

class Trader {
private:
    RequestInformation m_requestInformation;
    unsigned int m_traderNumber;
    void submitBuyOrder(const std::string& market, const double& quantity, const double& price);
    void submitSellOrder(const std::string& market, const double& quantity, const double& price);
    void submitCancelOrder(const std::string& market, const double& quantity, const double& price);
    std::string getOrders();
    std::string getBalance();
    std::string getBalances();

public:
    Trader::Trader(const unsigned int traderNumber, const std::string& host, const std::string& path, const double timeout) {
        m_requestInformation.host = host;
        m_requestInformation.path = path;
        m_requestInformation.timeout = timeout;
        m_traderNumber = traderNumber;
        std::cout << "Trader " << traderNumber << " created!\n";
    }
    Trader::~Trader() {};
};