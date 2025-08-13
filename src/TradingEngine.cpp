#include "TradingEngine.h"
#include <cstdlib>
#include <ctime>

#include "TradingEngine.h"
#include <cstdlib>
#include <ctime>

TradingEngine::TradingEngine()
    : portfolio_(100000.0), nextOrderID_(1), orderGenerator_(orderbook_) {
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
}

void TradingEngine::initialize() {
    orderbook_ = Orderbook();
    tradeHistory_.clear();
    portfolio_ = Portfolio(100000.0);
    portfolioValues_.clear();
    nextOrderID_ = 1;
}

void TradingEngine::runSimulation(int numIterations) {
    for (int i = 0; i < numIterations; ++i) {
        int numOrders = std::rand() % 10 + 1;
        std::vector<Order> orders = orderGenerator_.generateOrders(numOrders, nextOrderID_);

        for (auto& order : orders) {
            processOrder(order);
        }

        if (i % 10 == 0) {
            Quantity qty = 100; 
            Price price = orderbook_.getMidPrice();
            OrderSide side = (i % 20 == 0) ? OrderSide::SELL : OrderSide::BUY;
            OrderType type = OrderType::LIMIT;
            DurationType duration = DurationType::GOOD_TILL_CANCELLED;

            Order personalOrder(nextOrderID_++, qty, price, type, side, duration, true);
            processPersonalOrder(personalOrder);
        }

        Price currentPrice = orderbook_.getMidPrice();
        double portfolioValue = portfolio_.getPortfolioValue(currentPrice);
        portfolioValues_.push_back(portfolioValue);
    }
}

void TradingEngine::processOrder(Order& order) {
    TradeList trades = orderbook_.addOrder(order);
    tradeHistory_.insert(tradeHistory_.end(), trades.begin(), trades.end());
}

void TradingEngine::processPersonalOrder(Order& order) {
    TradeList trades = orderbook_.addOrder(order);
    tradeHistory_.insert(tradeHistory_.end(), trades.begin(), trades.end());
    portfolio_.update(trades);
}

const TradeList& TradingEngine::getTradeHistory() const {
    return tradeHistory_;
}

const std::vector<double>& TradingEngine::getPortfolioValues() const {
    return portfolioValues_;
}
