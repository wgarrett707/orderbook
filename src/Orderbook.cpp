#include "Orderbook.h"

// CLASS: TradeChild
TradeChild::TradeChild()
    : orderID(0), price(0.0), quantity(0), isPersonalOrder(false) {}

TradeChild::TradeChild(OrderID id, Price p, Quantity q, bool isPersonal)
    : orderID(id), price(p), quantity(q), isPersonalOrder(isPersonal) {}

// CLASS: Trade
Trade::Trade(const TradeChild& buyOrder, const TradeChild& sellOrder, Price executionPrice)
    : buyOrder_(buyOrder), sellOrder_(sellOrder), executionPrice_(executionPrice) { }

const TradeChild& Trade::getBuyOrder() const { return buyOrder_; }
const TradeChild& Trade::getSellOrder() const { return sellOrder_; }
Price Trade::getPrice() const { return executionPrice_; }
const Quantity Trade::getTradedQuantity() const { 
    return std::min(buyOrder_.quantity, sellOrder_.quantity); 
}

// CLASS: Orderbook

Order* Orderbook::getOrder(OrderID id) {
    auto it = orders.find(id);
    if (it == orders.end()) {
        return nullptr;
    }

    auto [orderIter, priceIter, side] = it->second;
    return &(*orderIter);
}

TradeList Orderbook::addOrder(Order& order) {
    TradeList trades;
    Quantity quantityLeft = order.getQuantity();

    if (order.getSide() == OrderSide::BUY) {
        // Handle FILL_OR_KILL orders
        if (order.getDuration() == DurationType::FILL_OR_KILL) {
            Quantity totalAvailable = 0;
            auto tempQuantityLeft = quantityLeft;
            auto askIter = asks.begin();
            while (tempQuantityLeft > 0 && askIter != asks.end() && 
                  (order.getType() == OrderType::MARKET || askIter->first <= order.getPrice())) {
                for (const Order& askOrder : askIter->second) {
                    totalAvailable += askOrder.getQuantity();
                    tempQuantityLeft -= askOrder.getQuantity();
                    if (totalAvailable >= order.getQuantity()) break;
                }
                if (totalAvailable >= order.getQuantity()) break;
                ++askIter;
            }
            if (totalAvailable < order.getQuantity()) {
                // Cannot fully fill, cancel order
                order.setStatus(OrderStatus::CANCELLED);
                return trades;
            }
        }

        // Process asks to match the buy order
        auto askIter = asks.begin();
        while (quantityLeft > 0 && askIter != asks.end() &&
               (order.getType() == OrderType::MARKET || askIter->first <= order.getPrice())) {
            std::list<Order>& askOrders = askIter->second;
            auto orderIter = askOrders.begin();

            while (quantityLeft > 0 && orderIter != askOrders.end()) {
                Order& askOrder = *orderIter;
                Quantity tradeQuantity = std::min(quantityLeft, askOrder.getQuantity());
                Price tradePrice = askOrder.getPrice();

                bool buyIsPersonal = order.getIsPersonalOrder();
                bool sellIsPersonal = askOrder.getIsPersonalOrder();
                TradeChild buySide(order.getOrderId(), tradePrice, tradeQuantity, buyIsPersonal);
                TradeChild sellSide(askOrder.getOrderId(), tradePrice, tradeQuantity, sellIsPersonal);

                // Record the trade
                trades.emplace_back(buySide, sellSide, tradePrice);

                // Update quantities and statuses
                quantityLeft -= tradeQuantity;
                order.setFilledQuantity(order.getFilledQuantity() + tradeQuantity);
                askOrder.setFilledQuantity(askOrder.getFilledQuantity() + tradeQuantity);

                if (tradeQuantity == askOrder.getQuantity()) {
                    askOrder.setStatus(OrderStatus::FILLED);
                    orders.erase(askOrder.getOrderId());
                    orderIter = askOrders.erase(orderIter);
                } else {
                    askOrder.setQuantity(askOrder.getQuantity() - tradeQuantity);
                    askOrder.setStatus(OrderStatus::PARTIALLY_FILLED);
                    ++orderIter;
                }

                if (quantityLeft == 0) {
                    break;
                }
            }

            if (askOrders.empty()) {
                askIter = asks.erase(askIter);
            } else {
                ++askIter;
            }

            if (quantityLeft == 0) {
                break;
            }
        }

        // Set order status
        if (order.getFilledQuantity() == order.getQuantity()) {
            order.setStatus(OrderStatus::FILLED);
        } else if (order.getFilledQuantity() > 0) {
            order.setStatus(OrderStatus::PARTIALLY_FILLED);
        } else {
            order.setStatus(OrderStatus::OPEN);
        }

        // Handle Duration Types
        if (quantityLeft > 0 && (order.getDuration() == DurationType::IMMEDIATE_OR_CANCEL ||
                                 order.getDuration() == DurationType::FILL_OR_KILL)) {
            // Do not add remaining quantity to order book
            if (order.getDuration() == DurationType::FILL_OR_KILL) {
                // Rollback any changes
                rollbackTrades(trades);
                order.setStatus(OrderStatus::CANCELLED);
                return TradeList(); // No trades
            } else {
                order.setStatus(order.getFilledQuantity() > 0 ? OrderStatus::PARTIALLY_FILLED : OrderStatus::CANCELLED);
                return trades;
            }
        }

        // Add remaining quantity to bids
        if (quantityLeft > 0) {
            order.setQuantity(quantityLeft);
            auto [priceIter, inserted] = bids.emplace(order.getPrice(), std::list<Order>());
            if (!inserted) {
                priceIter = bids.find(order.getPrice());
            }
            priceIter->second.push_back(order);
            // Store iterators in orders map
            auto orderIter = std::prev(priceIter->second.end());
            orders[order.getOrderId()] = OrderInfo{orderIter, priceIter, OrderSide::BUY};
            order.setStatus(order.getFilledQuantity() > 0 ? OrderStatus::PARTIALLY_FILLED : OrderStatus::OPEN);
        }

    } else if (order.getSide() == OrderSide::SELL) {
        if (order.getDuration() == DurationType::FILL_OR_KILL) {
            Quantity totalAvailable = 0;
            auto tempQuantityLeft = quantityLeft;
            auto bidIter = bids.begin();
            while (tempQuantityLeft > 0 && bidIter != bids.end() &&
                   (order.getType() == OrderType::MARKET || bidIter->first >= order.getPrice())) {
                for (const Order& bidOrder : bidIter->second) {
                    totalAvailable += bidOrder.getQuantity();
                    tempQuantityLeft -= bidOrder.getQuantity();
                    if (totalAvailable >= order.getQuantity()) break;
                }
                if (totalAvailable >= order.getQuantity()) break;
                ++bidIter;
            }
            if (totalAvailable < order.getQuantity()) {
                // Cannot fully fill, cancel order
                order.setStatus(OrderStatus::CANCELLED);
                return trades;
            }
        }

        // Process bids to match the sell order
        auto bidIter = bids.begin();
        while (quantityLeft > 0 && bidIter != bids.end() &&
               (order.getType() == OrderType::MARKET || bidIter->first >= order.getPrice())) {
            std::list<Order>& bidOrders = bidIter->second;
            auto orderIter = bidOrders.begin();

            while (quantityLeft > 0 && orderIter != bidOrders.end()) {
                Order& bidOrder = *orderIter;
                Quantity tradeQuantity = std::min(quantityLeft, bidOrder.getQuantity());
                Price tradePrice = bidOrder.getPrice();

                bool buyIsPersonal = bidOrder.getIsPersonalOrder();
                bool sellIsPersonal = order.getIsPersonalOrder();
                TradeChild buySide(bidOrder.getOrderId(), tradePrice, tradeQuantity, buyIsPersonal);
                TradeChild sellSide(order.getOrderId(), tradePrice, tradeQuantity, sellIsPersonal);

                // Record trade
                trades.emplace_back(buySide, sellSide, tradePrice);

                // Update quantities and statuses
                quantityLeft -= tradeQuantity;
                order.setFilledQuantity(order.getFilledQuantity() + tradeQuantity);
                bidOrder.setFilledQuantity(bidOrder.getFilledQuantity() + tradeQuantity);

                if (tradeQuantity == bidOrder.getQuantity()) {
                    bidOrder.setStatus(OrderStatus::FILLED);
                    orders.erase(bidOrder.getOrderId());
                    orderIter = bidOrders.erase(orderIter);
                } else {
                    bidOrder.setQuantity(bidOrder.getQuantity() - tradeQuantity);
                    bidOrder.setStatus(OrderStatus::PARTIALLY_FILLED);
                    ++orderIter;
                }

                if (quantityLeft == 0) {
                    break;
                }
            }

            if (bidOrders.empty()) {
                bidIter = bids.erase(bidIter);
            } else {
                ++bidIter;
            }

            if (quantityLeft == 0) {
                break;
            }
        }

        // Set order status
        if (order.getFilledQuantity() == order.getQuantity()) {
            order.setStatus(OrderStatus::FILLED);
        } else if (order.getFilledQuantity() > 0) {
            order.setStatus(OrderStatus::PARTIALLY_FILLED);
        } else {
            order.setStatus(OrderStatus::OPEN);
        }

        // Handle Duration Types
        if (quantityLeft > 0 && (order.getDuration() == DurationType::IMMEDIATE_OR_CANCEL ||
                                 order.getDuration() == DurationType::FILL_OR_KILL)) {
            // Do not add remaining quantity to order book
            if (order.getDuration() == DurationType::FILL_OR_KILL) {
                // Rollback any changes
                rollbackTrades(trades);
                order.setStatus(OrderStatus::CANCELLED);
                return TradeList(); // No trades
            } else {
                order.setStatus(order.getFilledQuantity() > 0 ? OrderStatus::PARTIALLY_FILLED : OrderStatus::CANCELLED);
                return trades;
            }
        }

        // Add remaining quantity to asks
        if (quantityLeft > 0) {
            order.setQuantity(quantityLeft);
            auto [priceIter, inserted] = asks.emplace(order.getPrice(), std::list<Order>());
            if (!inserted) {
                priceIter = asks.find(order.getPrice());
            }
            priceIter->second.push_back(order);
            // Store iterators in orders map
            auto orderIter = std::prev(priceIter->second.end());
            orders[order.getOrderId()] = OrderInfo{orderIter, priceIter, OrderSide::SELL};
            order.setStatus(order.getFilledQuantity() > 0 ? OrderStatus::PARTIALLY_FILLED : OrderStatus::OPEN);
        }
    }

    return trades;
}

// Helper function to rollback trades
void Orderbook::rollbackTrades(const TradeList& trades) {
    for (const auto& trade : trades) {
        // Rollback for buy order
        OrderID buyOrderID = trade.getBuyOrder().orderID;
        auto buyIt = orders.find(buyOrderID);
        if (buyIt != orders.end()) {
            Order& buyOrder = *(buyIt->second.orderIterator);
            Quantity tradeQty = trade.getTradedQuantity();
            buyOrder.setFilledQuantity(buyOrder.getFilledQuantity() - tradeQty);
            buyOrder.setQuantity(buyOrder.getQuantity() + tradeQty);
            buyOrder.setStatus(OrderStatus::OPEN);
        }

        // Rollback for sell order
        OrderID sellOrderID = trade.getSellOrder().orderID;
        auto sellIt = orders.find(sellOrderID);
        if (sellIt != orders.end()) {
            Order& sellOrder = *(sellIt->second.orderIterator);
            Quantity tradeQty = trade.getTradedQuantity();
            sellOrder.setFilledQuantity(sellOrder.getFilledQuantity() - tradeQty);
            sellOrder.setQuantity(sellOrder.getQuantity() + tradeQty);
            sellOrder.setStatus(OrderStatus::OPEN);
        }
    }
}

bool Orderbook::cancelOrder(OrderID& orderID) {
    auto it = orders.find(orderID);
    if (it == orders.end()) {
        return false;
    }

    auto [orderIter, priceIter, side] = it->second;
    priceIter->second.erase(orderIter);

    if (priceIter->second.empty()) {
        if (side == OrderSide::BUY) {
            bids.erase(priceIter);
        } else {
            asks.erase(priceIter);
        }
    }

    orders.erase(it);
    return true;
}

const std::map<Price, std::list<Order>, std::greater<>>& Orderbook::getBids() const { 
    return bids; 
}

const std::map<Price, std::list<Order>, std::less<>>& Orderbook::getAsks() const { 
    return asks; 
}

const Price Orderbook::getHighestBid() const {
    return bids.empty() ? Price() : bids.begin()->first;
}

const Price Orderbook::getLowestAsk() const {
    return asks.empty() ? Price() : asks.begin()->first;
}

const Price Orderbook::getMidPrice() const {
    if (bids.empty() || asks.empty()) {
        return Price();
    }
    return (getHighestBid() + getLowestAsk()) / 2;
}

const Quantity Orderbook::getBidInterest() const {
    Quantity sum = 0;
    for (const auto& bidPair : bids) {
        for (const Order& bidOrder : bidPair.second) {
            sum += bidOrder.getQuantity();
        }
    }
    return sum;
}

const Quantity Orderbook::getSellInterest() const {
    Quantity sum = 0;
    for (const auto& askPair : asks) {
        for (const Order& askOrder : askPair.second) {
            sum += askOrder.getQuantity();
        }
    }
    return sum;
}

const Quantity Orderbook::getNetInterest() const {
    return getBidInterest() - getSellInterest();
}