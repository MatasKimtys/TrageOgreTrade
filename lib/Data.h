#pragma once
#include <iostream>

struct Balance {
    std::string currency;
    bool requestStatus;
    double current;
    double available;
};

struct GetOrder {
    std::string uuid;
    int date;
    std::string type;
    std::string market;
    double price;
    double quantity;
    double fulfilled;
    GetOrder() {}
    GetOrder(
        const std::string& _uuid,
        const int& _date,
        const std::string& _type,
        const std::string& _market,
        const double& _price,
        const double& _quantity,
        const double& _fulfilled
    ) :
        uuid(_uuid),
        date(_date),
        type(_type),
        market(_market),
        price(_price),
        quantity(_quantity),
        fulfilled(_fulfilled) {}
};

struct OrderMarket {
    bool status;
    std::map<float, float> buyOrders;
    std::map<float, float> sellOrders;
};

struct Order {
    bool status;
    std::string uuid;
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
