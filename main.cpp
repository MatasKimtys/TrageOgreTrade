#include <iostream>
#include "lib/Trader.h"

int main(int, char**){
    std::cout << "Hello, from TradeOgreTrade!\n";
    const std::string host = "tradeogre.com";
    const std::string path = "/api/v1/markets";
    const double timeout = 10.0;
    Trader Trader(0, host, path, timeout);

}
