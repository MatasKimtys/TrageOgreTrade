#include <iostream>
#include "lib/Trader.cpp"
#include "ctime"
#include "chrono"

using Balance_t = std::tuple<std::string, bool, double, double>;
struct RunTime {
    std::time_t startTime;
    std::time_t endTime;
};

void printTimeTaken(RunTime runTime) {
    const std::chrono::seconds duration (runTime.endTime - runTime.startTime);
    const auto hours = std::chrono::duration_cast<std::chrono::hours>(duration);
    const auto minutes = std::chrono::duration_cast<std::chrono::minutes>(duration - hours);
    const auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration - hours - minutes);
    std::string output = "Total runtime: ";
        output += (hours.count() > 0) ? std::to_string(hours.count()) + " hour" + ((hours.count() == 1) ? " " : "s ") : "";
        output += (minutes.count() > 0) ? std::to_string(minutes.count()) + " minute" + ((minutes.count() == 1) ? " " : "s ") : "";
        output += std::to_string(seconds.count()) + " second" + ((seconds.count() == 1) ? " " : "s ");
    std::cout << output << "\n";
}

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
    RunTime runTime;
    runTime.startTime = std::time(nullptr);
    std::cout << "Hello, from TradeOgreTrade!\n";
    copyFilesToBuild("apikey.txt");
    const std::string host = "https://tradeogre.com/api/v1";
    Trader trader(0, host);
    trader.getOrderBook("QUBIC-USDT");
    runTime.endTime = std::time(nullptr);
    printTimeTaken(runTime);
}
