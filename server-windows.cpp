#include <iostream>
#include <string>
#include <sstream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <map>
#include <vector>
#include <algorithm>

// Include your Orderbook header
#include "Orderbook.h"

#pragma comment(lib, "Ws2_32.lib") // Link Winsock library

Orderbook book;
std::vector<Trade> tradeHistory;

std::string serializeOrderbookToJson(const Orderbook& book) {
    std::ostringstream ss;
    ss << "{ \"bids\": [";
    {
        bool firstPrice = true;
        for (auto& bidPair : book.getBids()) {
            Price p = bidPair.first;
            Quantity totalQty = 0;
            for (auto& ord : bidPair.second) {
                totalQty += ord.getQuantity();
            }
            if (!firstPrice) ss << ",";
            ss << "{ \"price\":" << p << ", \"quantity\":" << totalQty << "}";
            firstPrice = false;
        }
    }
    ss << "], \"asks\": [";
    {
        bool firstPrice = true;
        for (auto& askPair : book.getAsks()) {
            Price p = askPair.first;
            Quantity totalQty = 0;
            for (auto& ord : askPair.second) {
                totalQty += ord.getQuantity();
            }
            if (!firstPrice) ss << ",";
            ss << "{ \"price\":" << p << ", \"quantity\":" << totalQty << "}";
            firstPrice = false;
        }
    }
    ss << "] }";
    return ss.str();
}

std::string serializeTradesToJson(const std::vector<Trade>& trades) {
    std::ostringstream ss;
    ss << "[";
    for (size_t i = 0; i < trades.size(); ++i) {
        const Trade& t = trades[i];
        ss << "{"
           << "\"buyOrderId\":" << t.getBuyOrder().orderID << ","
           << "\"sellOrderId\":" << t.getSellOrder().orderID << ","
           << "\"price\":" << t.getPrice() << ","
           << "\"quantity\":" << t.getTradedQuantity()
           << "}";
        if (i + 1 < trades.size()) ss << ",";
    }
    ss << "]";
    return ss.str();
}

std::string read_request_body(SOCKET client_socket, size_t content_length) {
    std::string body;
    body.resize(content_length);
    size_t total_read = 0;
    while (total_read < content_length) {
        int r = recv(client_socket, &body[total_read], content_length - total_read, 0);
        if (r <= 0) break;
        total_read += r;
    }
    body.resize(total_read);
    return body;
}

size_t get_content_length(const std::string& request) {
    std::istringstream iss(request);
    std::string line;
    while (std::getline(iss, line) && line != "\r") {
        if (line.rfind("Content-Length:", 0) == 0) {
            std::string cl = line.substr(strlen("Content-Length:"));
            cl.erase(std::remove_if(cl.begin(), cl.end(), ::isspace), cl.end());
            return std::stoul(cl);
        }
    }
    return 0;
}

void handle_request(SOCKET client_socket) {
    char buffer[4096] = {0};
    int bytes_read = recv(client_socket, buffer, sizeof(buffer), 0);
    if (bytes_read <= 0) {
        closesocket(client_socket);
        return;
    }

    std::string request_str(buffer, bytes_read);
    std::istringstream request_stream(request_str);
    std::string method, path, http_version;
    request_stream >> method >> path >> http_version;

    if (method == "GET" && path == "/orderbook") {
        std::string orderbook_json = serializeOrderbookToJson(book);
        std::string response = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n" + orderbook_json;
        send(client_socket, response.c_str(), response.size(), 0);
    } else if (method == "GET" && path == "/trades") {
        std::string trades_json = serializeTradesToJson(tradeHistory);
        std::string response = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n" + trades_json;
        send(client_socket, response.c_str(), response.size(), 0);
    } else if (method == "POST" && path == "/addOrder") {
        size_t content_length = get_content_length(request_str);
        std::string body;
        if (content_length > 0) {
            body = read_request_body(client_socket, content_length);
        }

        // Parsing logic for JSON as before...

        // 200 Response
        std::string response = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n{\"status\": \"success\"}";
        send(client_socket, response.c_str(), response.size(), 0);
    } else {
        // 404
        std::string response = "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\nRoute not found!";
        send(client_socket, response.c_str(), response.size(), 0);
    }

    closesocket(client_socket);
}

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed\n";
        return 1;
    }

    SOCKET server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_socket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed\n";
        WSACleanup();
        return 1;
    }

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(8080);

    if (bind(server_socket, (sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed\n";
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    if (listen(server_socket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Listen failed\n";
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    std::cout << "Server is running on port 8080...\n";
    while (true) {
        sockaddr_in client_addr;
        int client_addr_len = sizeof(client_addr);
        SOCKET client_socket = accept(server_socket, (sockaddr*)&client_addr, &client_addr_len);
        if (client_socket == INVALID_SOCKET) {
            std::cerr << "Accept failed\n";
            continue;
        }

        handle_request(client_socket);
    }

    closesocket(server_socket);
    WSACleanup();
    return 0;
}
