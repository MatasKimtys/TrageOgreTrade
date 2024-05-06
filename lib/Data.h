// #include <nlohmann/json.hpp>

struct Balance {
    std::string currency;
    bool requestStatus;
    double current;
    double available;
};

class DataHolder
{
private:
    /* data */
public:
    // nlohmann::json getMarkets();
    // nlohmann::json getTickers();
    // nlohmann::json getTradeHistory();
    std::string listMarkets();
    DataHolder(/* args */);
    ~DataHolder();
};
