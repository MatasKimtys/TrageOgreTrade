#pragma once
#include <iostream>

struct Balance {
    std::string currency;
    bool requestStatus;
    double current;
    double available;
};

struct GetOrder {
    bool status;
    unsigned int date;
    std::string type;
    std::string market;
    double price;
    double quantity;
    double fulfilled;
};

struct OrderMarket {
    bool status;
    std::map<float, float> buyOrders;
    std::map<float, float> sellOrders;
};

struct Order {
    bool status;
    uuid_t uuid;
    double buyNewBalanceAvailable;
    double sellNewBalanceAvailable;
};

struct Market {
    std::string currency;
    double initialPrice;
    double price;
    double high;
    double low;
    double volume;
    double bid;
    double ask;
};

struct Ticker {
    bool status;
    double initialPrice;
    double price;
    double high;
    double low;
    double volume;
    double bid;
    double ask;
};

struct Trade {
    unsigned int date;
    std::string type;
    double price;
    double quanitity;
};

struct BuyOrder : Order {};
struct SellOrder : Order {};

class DataHolder
{
private:
    
public:
    DataHolder(/* args */);
    ~DataHolder();
    
};
