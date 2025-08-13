#include "Order.h"
#include <string>

extern "C" {
    const char* create_order(int orderId, int quantity, double price, const char* type, const char* side) {
        static std::string result;

        OrderType orderType;

        if (std::string(type) == "LIMIT") {
            orderType = OrderType::LIMIT;
        } else {
            orderType = OrderType::MARKET;
        }

        OrderSide orderSide;

        if (std::string(side) == "BUY") {
            orderSide = OrderSide::BUY;
        } else {
            orderSide = OrderSide::SELL;
        }

        Order order(orderId, quantity, price, orderType, orderSide, DurationType::GOOD_TILL_CANCELLED);

        result = "Order created: ID =" + std::to_string(order.getOrderId()) +
                 ", Quantity =" + std::to_string(order.getQuantity()) +
                 ", Price =" + std::to_string(order.getPrice()) +
                 ", Type =" + std::string(type) +
                 ", Side =" + std::string(side);

        return result.c_str();
    }
}