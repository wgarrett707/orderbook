#include <iostream>
#include <string>
#include <sstream>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <map>
#include <vector>
#include <algorithm>

#include "Orderbook.h"

Orderbook book;
std::vector<Trade> tradeHistory;

std::string serializeOrderbookToJson(const Orderbook& book) {
    // Build JSON string for { "bids": [ {price, quantity}, ... ], "asks": [...] }

    // Example:
    // {
    //   "bids": [
    //     { "price": 100.5, "quantity": 300 },
    //     ...
    //   ],
    //   "asks": [
    //     { "price": 101.0, "quantity": 200 },
    //     ...
    //   ]
    // }

    std::ostringstream ss;
    ss << "{ \"bids\": [";
    {
        bool firstPrice = true;
        for (auto &bidPair : book.getBids()) {
            Price p = bidPair.first;
            Quantity totalQty = 0;
            for (auto &ord : bidPair.second) {
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
        for (auto &askPair : book.getAsks()) {
            Price p = askPair.first;
            Quantity totalQty = 0;
            for (auto &ord : askPair.second) {
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
    for (size_t i=0; i<trades.size(); ++i) {
        const Trade& t = trades[i];
        ss << "{"
           << "\"buyOrderId\":" << t.getBuyOrder().orderID << ","
           << "\"sellOrderId\":" << t.getSellOrder().orderID << ","
           << "\"price\":" << t.getPrice() << ","
           << "\"quantity\":" << t.getTradedQuantity()
           << "}";
        if (i+1<trades.size()) ss << ",";
    }
    ss << "]";
    return ss.str();
}

std::string read_request_body(int client_fd, size_t content_length) {
    std::string body;
    body.resize(content_length);
    size_t total_read = 0;
    while (total_read < content_length) {
        ssize_t r = read(client_fd, &body[total_read], content_length - total_read);
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

void handle_request(int client_fd) {
    char buffer[4096] = {0};
    int bytes_read = read(client_fd, buffer, sizeof(buffer));
    if (bytes_read <= 0) {
        close(client_fd);
        return;
    }

    std::string request_str(buffer, bytes_read);
    std::istringstream request_stream(request_str);
    std::string method, path, http_version;
    request_stream >> method >> path >> http_version;

    if (method == "GET" && path == "/orderbook") {
        std::string orderbook_json = serializeOrderbookToJson(book);
        std::string response = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n" + orderbook_json;
        send(client_fd, response.c_str(), response.size(), 0);
    } else if (method == "GET" && path == "/trades") {
        std::string trades_json = serializeTradesToJson(tradeHistory);
        std::string response = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n" + trades_json;
        send(client_fd, response.c_str(), response.size(), 0);
    } else if (method == "POST" && path == "/addOrder") {
        size_t content_length = get_content_length(request_str);
        std::string body;
        if (content_length > 0) {
            body = read_request_body(client_fd, content_length);
        }

        // Parse JSON 
        // { "orderId": 123, "price":100.5, "quantity":200, "side":"BUY", "type":"LIMIT", "duration":"GOOD_TIL_CANCEL" }
        OrderID orderId = 0;
        Price price = 0.0;
        Quantity quantity = 0;
        std::string side_str, type_str, duration_str;
        bool isPersonal = false;

        {
            auto find_val = [&](const std::string& key) {
                std::string pattern = "\"" + key + "\"";
                size_t pos = body.find(pattern);
                if (pos == std::string::npos) return std::string();
                pos = body.find(":", pos);
                if (pos == std::string::npos) return std::string();
                size_t start = body.find_first_of("\"0123456789.-", pos+1);
                if (start == std::string::npos) return std::string();
                bool in_quotes = (body[start] == '"');
                size_t end;
                if (in_quotes) {
                    end = body.find("\"", start+1);
                    return body.substr(start+1, end - (start+1));
                } else {
                    end = body.find_first_of(",}", start+1);
                    return body.substr(start, end - start);
                }
            };
            std::cout << find_val("orderId") << std::endl;
            orderId = std::stoull(find_val("orderId"));
            price = std::stod(find_val("price"));
            quantity = std::stoul(find_val("quantity"));
            side_str = find_val("side");
            type_str = find_val("type");
            duration_str = find_val("duration");
            std::string isPersonalStr = find_val("isPersonalOrder");
            isPersonal = (isPersonalStr == "true");
        }

        // Convert strings to enums (You must define these conversions)
        OrderSide side = (side_str == "BUY") ? OrderSide::BUY : OrderSide::SELL;
        OrderType type = (type_str == "LIMIT") ? OrderType::LIMIT : OrderType::MARKET; // adjust as needed
        DurationType duration = DurationType::GOOD_TILL_CANCELLED;
        if (duration_str == "IMMEDIATE_OR_CANCEL") duration = DurationType::IMMEDIATE_OR_CANCEL;
        else if (duration_str == "FILL_OR_KILL") duration = DurationType::FILL_OR_KILL;

        // Construct the order and add it
        Order newOrder(orderId, quantity, price, type, side, duration, isPersonal);
        TradeList trades = book.addOrder(newOrder);

        // Append these trades to the global tradeHistory
        for (auto &t : trades) {
            tradeHistory.push_back(t);
        }

        // Return the updated orderbook and trades in one response
        std::string orderbook_json = serializeOrderbookToJson(book);
        std::string trades_json = serializeTradesToJson(tradeHistory);

        std::string response_body = "{ \"orderbook\": " + orderbook_json + ", \"trades\": " + trades_json + " }";

        std::string response = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n" + response_body;
        send(client_fd, response.c_str(), response.size(), 0);
    } else {
        // 404
        std::string response = "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\nRoute not found!";
        send(client_fd, response.c_str(), response.size(), 0);
    }

    close(client_fd);
}

int main() {
    std::cout << "Created Orderbook\n";

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        perror("Socket creation failed");
        return 1;
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        return 1;
    }

    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        return 1;
    }

    std::cout << "Server is running on port 8080...\n";
    while (true) {
        socklen_t addrlen = sizeof(address);
        int client_fd = accept(server_fd, (struct sockaddr*)&address, &addrlen);
        if (client_fd < 0) {
            perror("Accept failed");
            continue;
        }

        handle_request(client_fd);
    }

    close(server_fd);
    return 0;
}
