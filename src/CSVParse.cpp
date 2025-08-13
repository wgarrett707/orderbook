#include "CSVParse.h"

// csv expected to be like
// orderId, quantity, price,  type,  side,   DurationType::GOOD_TILL_CANCELLED
std::vector<Order> CSVParse::parseOrders(const std::string& fileName) {
    std::vector<Order> orders;
    std::ifstream file(fileName);

    if (!file.is_open()) {
        //file does not open
        throw std::runtime_error(fileName + " :failed to open and does not work");
    }

    std::string line;
    while (std::getline(file, line)) {
        std::stringstream info(line);
        std::string part;
        std::vector<std::string> order_data;

        while (std::getline(info, part, ',')) {
            // puts all comma sperated info from a line into order_data
            order_data.push_back(part); 
        }

        if (order_data.size() < 6) {
            // line did not contain all piece of info needed to create order
            throw std::invalid_argument("CSV line did not have all required information");
        }
        int ID = std::stoi(order_data[0]);
        int quantity = std::stoi(order_data[1]);
        double price = std::stod(order_data[2]);

        OrderType type;
        if (order_data[3] == "OrderType::MARKET") {
            type = OrderType::MARKET;
        } else { type = OrderType::LIMIT; }

        OrderSide side;
        if (order_data[4] == "OrderSide::BUY") {
            side = OrderSide::BUY;
        } else { side = OrderSide::SELL; }

        DurationType duration = DurationType::GOOD_TILL_CANCELLED;

        Order order(ID, quantity, price, type, side, duration);
        orders.push_back(order);
    }
    return orders;
}