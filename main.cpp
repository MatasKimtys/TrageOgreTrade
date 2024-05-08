#include <iostream>
#include "lib/Trader.cpp"

using Balance_t = std::tuple<std::string, bool, double, double>;

void copyFilesToBuild(const std::string filename) {
    std::filesystem::path currentWorkingDirectory = std::filesystem::current_path();
    std::filesystem::path sourcePath = std::filesystem::relative(currentWorkingDirectory) / ".." / filename;
    std::filesystem::path destinationPath =  currentWorkingDirectory / filename;
    try {
        if (!std::filesystem::exists(sourcePath)) {
            throw "Error: Source file does not exist.\n";
        }
        std::filesystem::copy_file(
            sourcePath, 
            destinationPath, 
            std::filesystem::copy_options::overwrite_existing
        );
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
    }
}

int main(int, char**){
    std::cout << "Hello, from TradeOgreTrade!\n";
    copyFilesToBuild("apikey.txt");
    const std::string host = "https://tradeogre.com/api/v1";
    Trader trader(0, host);
    trader.getSpecificMarketJson("QUBIC-USDT");

}
