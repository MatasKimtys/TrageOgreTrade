#include <iostream>
#include "lib/Trader.cpp"
#include "lib/Data.h"

using Balance_t = std::tuple<std::string, bool, double, double>;

int main(int, char**){
    std::cout << "Hello, from TradeOgreTrade!\n";
    const std::string host = "https://tradeogre.com/api/v1";
    Trader trader(0, host);
    trader.getSpecificMarketJson("QUBIC-USDT").wait();

}
