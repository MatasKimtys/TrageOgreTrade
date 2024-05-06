#include "Trader.h"


void Trader::submitBuyOrder(const std::string& market, const double& quantity, const double& price)
{
    std::string path {"/order/sell"}
}

void Trader::submitSellOrder(const std::string& market, const double& quantity, const double& price)
{
}

void Trader::submitCancelOrder(uuid uuid)
{
}

std::string Trader::getOrders()
{
    return std::string();
}

std::string Trader::getBalance()
{
    return std::string();
}

std::string Trader::getBalances()
{
    return std::string();
}
