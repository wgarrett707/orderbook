#include "Order.h"

Order::Order()
    : orderId(0), quantity(0), price(0.0), filledQuantity(0), type(OrderType::LIMIT),
      side(OrderSide::BUY), status(OrderStatus::OPEN), duration(DurationType::GOOD_TILL_CANCELLED),
      timestamp(0), expiryTime(0), isPersonalOrder(false) {}

Order::Order(OrderID orderId, Quantity quantity, Price price, OrderType type, OrderSide side, DurationType duration, bool isPersonalOrder)
    : orderId(orderId), quantity(quantity), price(price), filledQuantity(0), type(type), side(side),
      status(OrderStatus::OPEN), duration(duration), timestamp(0), expiryTime(0), isPersonalOrder(isPersonalOrder) {}

// Order class getters
OrderID Order::getOrderId() const { return orderId; }
Quantity Order::getQuantity() const { return quantity; }
Price Order::getPrice() const { return price; }
Quantity Order::getFilledQuantity() const { return filledQuantity; }
OrderType Order::getType() const { return type; }
OrderSide Order::getSide() const { return side; }
OrderStatus Order::getStatus() const { return status; }
Timestamp Order::getTimestamp() const { return timestamp; }
ExpiryTime Order::getExpiryTime() const { return expiryTime; }
DurationType Order::getDuration() const { return duration; }
bool Order::getIsPersonalOrder() const { return isPersonalOrder; }

// Order class setters
void Order::setOrderId(OrderID id) { orderId = id; }
void Order::setQuantity(Quantity newQuantity) { quantity = newQuantity; }
void Order::setStatus(OrderStatus newStatus) { status = newStatus; }
void Order::setFilledQuantity(Quantity filledQty) { filledQuantity = filledQty; }
void Order::setExpiryTime(ExpiryTime expiry) { expiryTime = expiry; }
void Order::setIsPersonalOrder(bool isPersonal) { isPersonalOrder = isPersonal; }