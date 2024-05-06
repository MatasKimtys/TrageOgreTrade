cmake_minimum_required(VERSION 3.1)
project(TradeOgreTrade VERSION 0.1.0 LANGUAGES C CXX)

find_package(cpprestsdk REQUIRED)
find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})
add_executable(TradeOgreTrade main.cpp)

target_link_libraries(TradeOgreTrade PUBLIC uuid cpprestsdk::cpprest ${OPENSSL_LIBRARIES})

