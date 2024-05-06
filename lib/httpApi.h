#include <iostream>
#include <string>

std::string httpRequest(
    const std::string& host,
    const std::string& path,
    const std::string& method,
    const std::string& body = "",
    const int timeout_seconds = 10
) {

    std::ostringstream requestStream;
    requestStream << method << " " << path << " HTTP/1.1\r\n"
        << "Host: " << host << "\r\n"
        << "Content-Length: " << body.length() << "\r\n"
        << "Connection: close\r\n";
}

std::string getResponce(const std::string& host, const std::string& path) {
    std::string response = httpRequest(host, path, "GET");
    std::cout << "GET Response:\n" << response << std::endl;
    return response
}