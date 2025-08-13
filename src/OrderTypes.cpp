#include "OrderTypes.h"
#include "types.h"

// Constructor for MarketOrder
MarketOrder::MarketOrder(OrderID orderId, Quantity quantity, OrderSide side, bool isPersonalOrder)
    : Order(orderId, quantity, 0.0, OrderType::MARKET, side, DurationType::GOOD_TILL_CANCELLED, isPersonalOrder) {}

// Constructor for LimitOrder
LimitOrder::LimitOrder(OrderID orderId, Quantity quantity, Price price, OrderSide side, bool isPersonalOrder)
    : Order(orderId, quantity, price, OrderType::LIMIT, side, DurationType::GOOD_TILL_CANCELLED, isPersonalOrder) {}
