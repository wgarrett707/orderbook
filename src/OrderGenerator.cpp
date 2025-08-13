#include "OrderGenerator.h"
#include <cstdlib>
#include <cmath>

OrderGenerator::OrderGenerator(Orderbook& orderbook)
    : orderbook_(orderbook) {}

void OrderGenerator::setType(OrderType type) {
    type_ = type;
}

std::vector<Order> OrderGenerator::generateOrders(int num_orders, OrderID& nextOrderID) {
    std::vector<Order> orders;
    Price midPrice = orderbook_.getMidPrice();

    if (midPrice <= 0.0) {
        midPrice = defaultPrice_;
    }

    for (int i = 0; i < num_orders; ++i) {
        Quantity qty = std::rand() % 100 + 1;
        Price price = getRandomPriceAroundMid(midPrice);
        OrderSide side = (std::rand() % 2 == 0) ? OrderSide::BUY : OrderSide::SELL; // randomly select buy or sell
        DurationType duration = DurationType::GOOD_TILL_CANCELLED;

        Order order(nextOrderID++, qty, price, type_, side, duration, false);
        orders.push_back(order);
    }
    return orders;
}

std::vector<Order> OrderGenerator::generateOrdersFixedRange(int num_orders, OrderID& nextOrderID) {
    std::vector<Order> orders;
    Price midPrice = orderbook_.getMidPrice();

    if (midPrice <= 0.0) {
        midPrice = defaultPrice_;
    }

    for (int i = 0; i < num_orders; ++i) {
        Quantity qty = std::rand() % 100 + 1;
        Price price = std::rand() % 10000;
        OrderSide side = (std::rand() % 2 == 0) ? OrderSide::BUY : OrderSide::SELL; // randomly select buy or sell
        DurationType duration = DurationType::GOOD_TILL_CANCELLED;

        Order order(nextOrderID++, qty, price, type_, side, duration, false);
        orders.push_back(order);
    }
    return orders;
}

Price OrderGenerator::getRandomPriceAroundMid(Price midPrice) {
    double percentageChange = ((std::rand() % 2001) - 1000) / 100000.0; // -1% to +1%
    Price price = midPrice * (1 + percentageChange);
    price = std::round(price * 100.0) / 100.0;

    if (price <= 0.0) {
        price = defaultPrice_;
    }

    return price;
}
