#include "gtest/gtest.h"
#include <iostream>
#include "OrderTypes.h"
#include "Orderbook.h"
#include "Order.h"
#include "types.h"
#include "OrderGenerator.h"
#include "CSVParse.h"
#include "Portfolio.h"
#include "TradingEngine.h"

TEST(BasicTests, Multiplication) {
    int one = 1;
    int two = 2;
    ASSERT_EQ(one * two, 2);
}

// Test for MarketOrder getters
TEST(MarketOrderTests, Getters) {
    MarketOrder order1(1, 10, OrderSide::BUY, true);
    EXPECT_EQ(order1.getOrderId(), 1);
    EXPECT_EQ(order1.getQuantity(), 10);
    EXPECT_EQ(order1.getStatus(), OrderStatus::OPEN);
    EXPECT_EQ(order1.getSide(), OrderSide::BUY);
    EXPECT_EQ(order1.getType(), OrderType::MARKET);
    EXPECT_EQ(order1.getIsPersonalOrder(), true);
}

// Test for LimitOrder getters
TEST(LimitOrderTests, Getters) {
    LimitOrder order1(1, 10, 100.0, OrderSide::BUY, false);
    EXPECT_EQ(order1.getOrderId(), 1);
    EXPECT_EQ(order1.getQuantity(), 10);
    EXPECT_EQ(order1.getPrice(), 100.0);
    EXPECT_EQ(order1.getStatus(), OrderStatus::OPEN);
    EXPECT_EQ(order1.getSide(), OrderSide::BUY);
    EXPECT_EQ(order1.getType(), OrderType::LIMIT);
    EXPECT_EQ(order1.getIsPersonalOrder(), false);
}

// Test for MarketOrder setters
TEST(MarketOrderTests, Setters) {
    MarketOrder order(1, 10, OrderSide::BUY);
    order.setStatus(OrderStatus::FILLED);
    EXPECT_EQ(order.getStatus(), OrderStatus::FILLED);
    order.setFilledQuantity(5);
    EXPECT_EQ(order.getFilledQuantity(), 5);
    order.setIsPersonalOrder(true);
    EXPECT_EQ(order.getIsPersonalOrder(), true);
    EXPECT_EQ(order.getOrderId(), 1);
}

// Test for LimitOrder setters
TEST(LimitOrderTests, Setters) {
    LimitOrder order(1, 10, 245.00, OrderSide::BUY);
    order.setStatus(OrderStatus::FILLED);
    EXPECT_EQ(order.getStatus(), OrderStatus::FILLED);
    order.setFilledQuantity(5);
    EXPECT_EQ(order.getFilledQuantity(), 5);
    order.setIsPersonalOrder(true);
    EXPECT_EQ(order.getIsPersonalOrder(), true);
    EXPECT_EQ(order.getOrderId(), 1);
}

// Orderbook and LimitOrder tests
TEST(OrderbookTests, SingleLimitOrder1) {
    Orderbook book;
    LimitOrder buyOrder(1, 10, 90.00, OrderSide::BUY);
    TradeList trades = book.addOrder(buyOrder);
    EXPECT_EQ(buyOrder.getStatus(), OrderStatus::OPEN);
    EXPECT_EQ(book.getBidInterest(), 10);
    EXPECT_EQ(book.getHighestBid(), 90.00);
    EXPECT_EQ(book.getSellInterest(), 0);
    EXPECT_EQ(book.getLowestAsk(), Price());
    EXPECT_EQ(book.getNetInterest(), 10);
    EXPECT_EQ(trades.size(), 0);
}

// Basic matching prices test
TEST(OrderbookTests, BasicMatchingPrices) {
    Orderbook book;
    Order buyOrder(1, 1, 100, OrderType::LIMIT, OrderSide::BUY, DurationType::GOOD_TILL_CANCELLED);
    Order sellOrder(2, 1, 100, OrderType::LIMIT, OrderSide::SELL, DurationType::GOOD_TILL_CANCELLED);
    TradeList trades1 = book.addOrder(buyOrder);
    TradeList trades2 = book.addOrder(sellOrder);

    EXPECT_EQ(book.getHighestBid(), Price());
    EXPECT_EQ(book.getLowestAsk(), Price());
    EXPECT_EQ(trades2.size(), 1);

    const Trade& trade = trades2.front();
    EXPECT_EQ(trade.getTradedQuantity(), 1);
    EXPECT_EQ(trade.getPrice(), 100);
    EXPECT_EQ(trade.getBuyOrder().orderID, 1);
    EXPECT_EQ(trade.getSellOrder().orderID, 2);
}

// Basic bid interest test
TEST(OrderbookTests, BasicBidInterest) {
    Orderbook book;
    Order buyOrder1(1, 1, 40, OrderType::LIMIT, OrderSide::BUY, DurationType::GOOD_TILL_CANCELLED);
    Order buyOrder2(2, 1, 50, OrderType::LIMIT, OrderSide::BUY, DurationType::GOOD_TILL_CANCELLED);
    Order buyOrder3(3, 1, 60, OrderType::LIMIT, OrderSide::BUY, DurationType::GOOD_TILL_CANCELLED);
    book.addOrder(buyOrder1);
    book.addOrder(buyOrder2);
    book.addOrder(buyOrder3);
    EXPECT_EQ(book.getBidInterest(), 3);
    EXPECT_EQ(book.getHighestBid(), 60);
}

// Test for partial fill when sell quantity is less than buy quantity
TEST(OrderbookTests, PartialFillBuyOrder) {
    Orderbook book;
    LimitOrder buyOrder(1, 10, 100, OrderSide::BUY);
    book.addOrder(buyOrder);
    LimitOrder sellOrder(2, 5, 100, OrderSide::SELL);
    TradeList trades = book.addOrder(sellOrder);

    EXPECT_EQ(book.getBidInterest(), 5);
    EXPECT_EQ(book.getSellInterest(), 0);
    EXPECT_EQ(trades.size(), 1);

    const Trade& trade = trades.front();
    EXPECT_EQ(trade.getTradedQuantity(), 5);
    EXPECT_EQ(trade.getPrice(), 100);
    EXPECT_EQ(trade.getBuyOrder().orderID, 1);
    EXPECT_EQ(trade.getSellOrder().orderID, 2);
}

// Test for multiple matching orders
TEST(OrderbookTests, MultipleMatchOrders) {
    Orderbook book;
    LimitOrder buyOrder1(1, 5, 100, OrderSide::BUY);
    LimitOrder buyOrder2(2, 5, 100, OrderSide::BUY);
    book.addOrder(buyOrder1);
    book.addOrder(buyOrder2);
    LimitOrder sellOrder(3, 10, 100, OrderSide::SELL);
    TradeList trades = book.addOrder(sellOrder);

    EXPECT_EQ(book.getBidInterest(), 0);
    EXPECT_EQ(book.getSellInterest(), 0);
    EXPECT_EQ(trades.size(), 2);

    EXPECT_EQ(trades[0].getBuyOrder().orderID, 1);
    EXPECT_EQ(trades[1].getBuyOrder().orderID, 2);
}

// Test for FIFO matching at the same price
TEST(OrderbookTests, FIFOOrderMatching) {
    Orderbook book;
    LimitOrder buyOrder1(1, 5, 100, OrderSide::BUY);
    LimitOrder buyOrder2(2, 5, 100, OrderSide::BUY);
    book.addOrder(buyOrder1);
    book.addOrder(buyOrder2);
    LimitOrder sellOrder(3, 5, 100, OrderSide::SELL);
    TradeList trades = book.addOrder(sellOrder);

    EXPECT_EQ(book.getBidInterest(), 5);
    EXPECT_EQ(book.getSellInterest(), 0);
    EXPECT_EQ(trades.size(), 1);

    const Trade& trade = trades.front();
    EXPECT_EQ(trade.getBuyOrder().orderID, 1);
}

// Test for order cancellation
TEST(OrderbookTests, OrderCancellation) {
    Orderbook book;
    LimitOrder buyOrder(1, 10, 100, OrderSide::BUY);
    book.addOrder(buyOrder);
    EXPECT_EQ(book.getBidInterest(), 10);
    OrderID orderID = buyOrder.getOrderId();
    bool cancelResult = book.cancelOrder(orderID);
    EXPECT_TRUE(cancelResult);
    EXPECT_EQ(book.getBidInterest(), 0);
    EXPECT_EQ(book.getOrder(orderID), nullptr);
}

// Test for canceling a non-existent order
TEST(OrderbookTests, CancelNonExistentOrder) {
    Orderbook book;
    OrderID nonExistentOrderID = 999;
    bool cancelResult = book.cancelOrder(nonExistentOrderID);
    EXPECT_FALSE(cancelResult);
}

// Test for market order matching immediately
TEST(OrderbookTests, MarketOrderMatching) {
    Orderbook book;
    LimitOrder sellOrder(1, 10, 100, OrderSide::SELL);
    book.addOrder(sellOrder);
    MarketOrder buyOrder(2, 5, OrderSide::BUY);
    TradeList trades = book.addOrder(buyOrder);

    EXPECT_EQ(book.getBidInterest(), 0);
    EXPECT_EQ(book.getSellInterest(), 5);
    EXPECT_EQ(trades.size(), 1);

    const Trade& trade = trades.front();
    EXPECT_EQ(trade.getTradedQuantity(), 5);
    EXPECT_EQ(trade.getPrice(), 100);
}

// Test for market order with no liquidity
TEST(OrderbookTests, MarketOrderNoLiquidity) {
    Orderbook book;
    MarketOrder buyOrder(1, 5, OrderSide::BUY);
    TradeList trades = book.addOrder(buyOrder);

    EXPECT_EQ(book.getBidInterest(), 5);
    EXPECT_EQ(book.getSellInterest(), 0);
    EXPECT_EQ(trades.size(), 0);
}

// Test for Immediate-Or-Cancel order with partial fill
TEST(OrderbookTests, ImmediateOrCancelOrderPartialFill) {
    Orderbook book;
    LimitOrder sellOrder(1, 5, 100, OrderSide::SELL);
    book.addOrder(sellOrder);
    Order buyOrder(2, 10, 100, OrderType::LIMIT, OrderSide::BUY, DurationType::IMMEDIATE_OR_CANCEL);
    TradeList trades = book.addOrder(buyOrder);

    EXPECT_EQ(book.getBidInterest(), 0);
    EXPECT_EQ(book.getSellInterest(), 0);
    EXPECT_EQ(trades.size(), 1);
    EXPECT_EQ(buyOrder.getStatus(), OrderStatus::PARTIALLY_FILLED);
    EXPECT_EQ(buyOrder.getFilledQuantity(), 5);
}

TEST(OrderbookTests, ImmediateOrCancelOrderFullFill) {
    Orderbook book;
    LimitOrder sellOrder(1, 10, 100, OrderSide::SELL);
    book.addOrder(sellOrder);
    Order buyOrder(2, 10, 100, OrderType::LIMIT, OrderSide::BUY, DurationType::IMMEDIATE_OR_CANCEL);
    TradeList trades = book.addOrder(buyOrder);

    EXPECT_EQ(book.getBidInterest(), 0);
    EXPECT_EQ(book.getSellInterest(), 0);
    EXPECT_EQ(trades.size(), 1);
    EXPECT_EQ(buyOrder.getStatus(), OrderStatus::FILLED);
    EXPECT_EQ(buyOrder.getFilledQuantity(), 10);
}

TEST(OrderbookTests, ImmediateOrCancelOrderNoFill) {
    Orderbook book;
    Order buyOrder(1, 10, 100, OrderType::LIMIT, OrderSide::BUY, DurationType::IMMEDIATE_OR_CANCEL);
    TradeList trades = book.addOrder(buyOrder);

    EXPECT_EQ(book.getBidInterest(), 0);
    EXPECT_EQ(trades.size(), 0);
    EXPECT_EQ(buyOrder.getStatus(), OrderStatus::CANCELLED);
}

TEST(OrderbookTests, FillOrKillOrderFullFill) {
    Orderbook book;
    LimitOrder sellOrder1(1, 5, 100, OrderSide::SELL);
    LimitOrder sellOrder2(2, 5, 100, OrderSide::SELL);
    book.addOrder(sellOrder1);
    book.addOrder(sellOrder2);
    Order buyOrder(3, 10, 100, OrderType::LIMIT, OrderSide::BUY, DurationType::FILL_OR_KILL);
    TradeList trades = book.addOrder(buyOrder);

    EXPECT_EQ(book.getBidInterest(), 0);
    EXPECT_EQ(book.getSellInterest(), 0);
    EXPECT_EQ(trades.size(), 2);
    EXPECT_EQ(buyOrder.getStatus(), OrderStatus::FILLED);
    EXPECT_EQ(buyOrder.getFilledQuantity(), 10);
}

TEST(OrderbookTests, FillOrKillOrderInsufficientLiquidity) {
    Orderbook book;
    LimitOrder sellOrder(1, 5, 100, OrderSide::SELL);
    book.addOrder(sellOrder);
    Order buyOrder(2, 10, 100, OrderType::LIMIT, OrderSide::BUY, DurationType::FILL_OR_KILL);
    TradeList trades = book.addOrder(buyOrder);

    EXPECT_EQ(book.getBidInterest(), 0);
    EXPECT_EQ(book.getSellInterest(), 5);
    EXPECT_EQ(trades.size(), 0);
    EXPECT_EQ(buyOrder.getStatus(), OrderStatus::CANCELLED);
}

// Test for net interest calculation
TEST(OrderbookTests, NetInterestCalculation) {
    Orderbook book;
    LimitOrder buyOrder1(1, 5, 100, OrderSide::BUY);
    LimitOrder buyOrder2(2, 10, 99, OrderSide::BUY);
    LimitOrder sellOrder1(3, 3, 101, OrderSide::SELL);
    LimitOrder sellOrder2(4, 5, 102, OrderSide::SELL);
    book.addOrder(buyOrder1);
    book.addOrder(buyOrder2);
    book.addOrder(sellOrder1);
    book.addOrder(sellOrder2);

    EXPECT_EQ(book.getBidInterest(), 15);
    EXPECT_EQ(book.getSellInterest(), 8);
    EXPECT_EQ(book.getNetInterest(), 7);
}

// Test for mid-price calculation
TEST(OrderbookTests, MidPriceCalculation) {
    Orderbook book;
    LimitOrder buyOrder(1, 10, 99, OrderSide::BUY);
    LimitOrder sellOrder(2, 10, 101, OrderSide::SELL);
    book.addOrder(buyOrder);
    book.addOrder(sellOrder);
    EXPECT_EQ(book.getMidPrice(), 100);
}

// Test for mid-price when order book is empty
TEST(OrderbookTests, MidPriceEmptyOrderbook) {
    Orderbook book;
    EXPECT_EQ(book.getMidPrice(), Price());
}

// Test to see if we can Parse a single order from csv
TEST(CSVParseTests, ParseSingleOrder) {
    CSVParse parser;
    std::string fileName = "parse_simple.csv";

    std::vector<Order> orders = parser.parseOrders(fileName);
    Order order = orders[0];
    EXPECT_EQ(order.getOrderId(), 1);
    EXPECT_EQ(order.getQuantity(), 5);
    EXPECT_DOUBLE_EQ(order.getPrice(), 100.0);
    EXPECT_EQ(order.getType(), OrderType::MARKET);
    EXPECT_EQ(order.getSide(), OrderSide::BUY);
    EXPECT_EQ(order.getDuration(), DurationType::GOOD_TILL_CANCELLED);
}

// Test to see if we can Parse a Multiple Orders from csv
TEST(CSVParseTests, ParseMultipleOrders) {
    CSVParse parser;
    std::string fileName = "parse_multiple.csv";

    std::vector<Order> orders = parser.parseOrders(fileName);

    ASSERT_EQ(orders.size(), 10);

    Order order1 = orders[0];
    EXPECT_EQ(order1.getOrderId(), 1);
    EXPECT_EQ(order1.getQuantity(), 5);
    EXPECT_DOUBLE_EQ(order1.getPrice(), 100.0);
    EXPECT_EQ(order1.getType(), OrderType::MARKET);
    EXPECT_EQ(order1.getSide(), OrderSide::BUY);
    EXPECT_EQ(order1.getDuration(), DurationType::GOOD_TILL_CANCELLED);

    Order order2 = orders[1];
    EXPECT_EQ(order2.getOrderId(), 2);
    EXPECT_EQ(order2.getQuantity(), 10);
    EXPECT_DOUBLE_EQ(order2.getPrice(), 105.0);
    EXPECT_EQ(order2.getType(), OrderType::MARKET);
    EXPECT_EQ(order2.getSide(), OrderSide::BUY);
}


TEST(RandomGeneratorTests, GeneratorCorrectCount) {
    Orderbook book;
    OrderGenerator order_generator(book);
    OrderID nextOrderID = 1;
    std::vector<Order> my_orders = order_generator.generateOrders(10, nextOrderID);
    EXPECT_EQ(my_orders.size(), 10);
}

TEST(RandomGeneratorTests, GeneratorPricesAroundMid) {
    Orderbook book;
    LimitOrder buyOrder(1, 10, 100, OrderSide::BUY);
    LimitOrder sellOrder(2, 10, 102, OrderSide::SELL);
    book.addOrder(buyOrder);
    book.addOrder(sellOrder);

    OrderGenerator order_generator(book);
    OrderID nextOrderID = 3;
    std::vector<Order> my_orders = order_generator.generateOrders(10, nextOrderID);
    Price midPrice = book.getMidPrice();

    for (const auto& order : my_orders) {
        Price price = order.getPrice();
        EXPECT_NEAR(price, midPrice, midPrice * 0.01);
    }
}

TEST(RandomGeneratorTests, GeneratorMidPriceChanges) {
    Orderbook book;
    LimitOrder buyOrder(1, 10, 100, OrderSide::BUY);
    LimitOrder sellOrder(2, 10, 102, OrderSide::SELL);
    book.addOrder(buyOrder);
    book.addOrder(sellOrder);

    OrderGenerator order_generator(book);
    OrderID nextOrderID = 3;
    std::vector<Order> my_orders = order_generator.generateOrders(10, nextOrderID);
    Price midPrice = book.getMidPrice();

    for (const auto& order : my_orders) {
        Price price = order.getPrice();
        EXPECT_NE(0, midPrice-price); // check that the price changes every time
    }
}

TEST(RandomGeneratorTests, GeneratorMidPriceChangesFixedRange) {
    Orderbook book;
    LimitOrder buyOrder(1, 10, 100, OrderSide::BUY);
    LimitOrder sellOrder(2, 10, 102, OrderSide::SELL);
    book.addOrder(buyOrder);
    book.addOrder(sellOrder);

    OrderGenerator order_generator(book);
    OrderID nextOrderID = 3;
    std::vector<Order> my_orders = order_generator.generateOrdersFixedRange(10, nextOrderID);
    Price midPrice = book.getMidPrice();

    for (const auto& order : my_orders) {
        Price price = order.getPrice();
        EXPECT_NE(0, midPrice-price); // check that the price changes every time
    }
}

TEST(TradingEngineTests, TradingEngineInitTest) {
    TradingEngine my_engine;
    EXPECT_EQ(my_engine.portfolio_, 100000.0);
    EXPERT_EQ(my_engine.nextOrderID_, 1);
}

TEST(TradingEngineTests, TradingEngineBasicSimulation) {
    TradingEngine my_engine;
    my_engine.runSimulation(10);
    std::vector<double>& portfolio_vals = my_engine.getPortfolioValues();
    EXPECT_EQ(portfolio_vals.size(), 10);
}

TEST(TradingEngineTests, TradingEngineResetsState) {
    TradingEngine my_engine;

    my_engine.runSimulation(10);
    my_engine.initialize();

    EXPECT_EQ(my_engine.getTradeHistory().size(), 0);
    EXPECT_EQ(my_engine.getPortfolioValues().size(), 0);
    EXPECT_EQ(my_engine.getPortfolioValues().empty(), true);
    EXPECT_EQ(my_engine.getTradeHistory().empty(), true);
}

TEST(TradingEngineTests, TradingEngineBasicTradeHistory) {
    TradingEngine my_engine;

    Order testOrder(1, 100, 50.0, OrderType::LIMIT, OrderSide::BUY, DurationType::GOOD_TILL_CANCELLED, false);
    my_engine.processOrder(testOrder);
    const TradeList& tradeHistory = my_engine.getTradeHistory();

    EXPECT_GT(tradeHistory.size(), 1); // tradehistory is a tradelist, which is a vector, so size should work
}

TEST(TradingEngineTests, TradingEngineBasicMatchOrders) {
    TradingEngine my_engine;

    my_engine.runSimulation(10);
    const TradeList& tradeHistory = my_engine.getTradeHistory();

    EXPECT_GT(tradeHistory.size(), 10); 
}

TEST(TradingEngineTests, TradingEngineNextOrderIDIncrement) {
    TradingEngine my_engine;

    int initialOrderID = 1;

    for (int i = 0; i < 5; ++i) {
        Order order(my_engine.nextOrderID_, 50, 100.0, OrderType::LIMIT, OrderSide::BUY, DurationType::GOOD_TILL_CANCELLED, false);
        my_engine.processOrder(order);
    }

    EXPECT_EQ(my_engine.nextOrderID_, initialOrderID + 5);
}


int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
