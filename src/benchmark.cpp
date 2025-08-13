#include <iostream>
#include <chrono>
#include "OrderTypes.h"
#include "Orderbook.h"
#include "OrderGenerator.h"

using namespace std;
using namespace std::chrono;

// measures time for 3000 orders in nanoseconds
void benchmarkMatch3000Orders() {
    Orderbook book;
    OrderGenerator generator;
    OrderID id = 0;
    auto orders = generator.generateOrders(100, id);

    auto start = high_resolution_clock::now();
    for (auto &order : orders) {
        book.addOrder(order);
    }
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(end - start).count();

    cout << "Time taken to match 3,000 orders (NASDAQ's average volume per second): " << (float)duration / 1000000 << " s" << endl;
}

// measures time for a million orders in milliseconds (then convert to seconds)
void benchmarkMatchMillionOrders() {
    Orderbook book;
    OrderGenerator generator;
    OrderID id = 0;
    auto orders = generator.generateOrders(1000000, id);

    auto start = high_resolution_clock::now();
    for (auto &order : orders) {
        book.addOrder(order);
    }
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start).count();

    cout << "Time taken to match 1,000,000 orders: " << (float)duration / 1000 << " s" << endl;
}

// measures number of orders processed in 5 seconds
void benchmarkOrdersProcessedIn5Seconds() {
    Orderbook book;
    OrderGenerator generator;
    OrderID id = 0;

    auto start = high_resolution_clock::now();
    int orderCount = 0;
    while (duration_cast<seconds>(high_resolution_clock::now() - start).count() < 5) {
        auto orders = generator.generateOrders(1, id); // maybe this shouldn't be part of the benchmark...
        book.addOrder(orders[0]);
        orderCount++;
    }

    cout << "Orders processed in 5 seconds: " << orderCount << endl;
}

// measures number of orders processed in a second
void benchmarkOrdersProcessedIn1Second() {
    Orderbook book;
    OrderGenerator generator;
    OrderID id = 0;

    auto start = high_resolution_clock::now();
    int orderCount = 0;
    while (duration_cast<seconds>(high_resolution_clock::now() - start).count() < 1) {
        auto orders = generator.generateOrders(1, id);
        book.addOrder(orders[0]);
        orderCount++;
    }

    cout << "Orders processed in 1 second: " << orderCount << endl;
}

// measures number of orders processed in a single millisecond
void benchmarkOrdersProcessedIn1Millisecond() {
    Orderbook book;
    OrderGenerator generator;
    OrderID id = 0;

    auto start = high_resolution_clock::now();
    int orderCount = 0;
    while (duration_cast<milliseconds>(high_resolution_clock::now() - start).count() < 1) {
        auto orders = generator.generateOrders(1, id);
        book.addOrder(orders[0]);
        orderCount++;
    }

    cout << "Orders processed in 1 millisecond: " << orderCount << endl;
}

// measures how fast a single order can be added
void benchmarkAddSingleOrder() {
    Orderbook book;
    OrderGenerator generator;
    OrderID id = 0;
    auto orders = generator.generateOrders(1, id);

    auto start = high_resolution_clock::now();
    book.addOrder(orders[0]);
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<nanoseconds>(end - start).count();

    cout << "Time taken to add a single order: " << duration << " ns" << endl;
}

// measures how fast a single order can be canceled
void benchmarkCancelSingleOrder() {
    Orderbook book;
    LimitOrder order(1, 10, 100.0, OrderSide::BUY);
    book.addOrder(order);
    OrderID orderId = order.getOrderId();

    auto start = high_resolution_clock::now();
    bool success = book.cancelOrder(orderId);
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<nanoseconds>(end - start).count();

    if (success) {
        cout << "Time taken to cancel a single order: " << duration << " ns" << endl;
    } else {
        cout << "Failed to cancel order!" << endl;
    }
}

// performs a MIX of half market orders and half limit orders
void benchmarkMixedOrderMatching(int numOrders) {
    Orderbook book;
    OrderGenerator generator;
    OrderID idLimit = 0;
    OrderID idMarket = numOrders / 2;
    auto limitOrders = generator.generateOrders(numOrders / 2, idLimit);
    generator.setType(OrderType::MARKET);
    auto marketOrders = generator.generateOrders(numOrders / 2, idMarket);

    auto start = high_resolution_clock::now();
    for (auto &order : limitOrders) {
        book.addOrder(order);
    }
    for (auto &order : marketOrders) {
        book.addOrder(order);
    }
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start).count();
    cout << "Time taken to match " << numOrders << " mixed orders: " << (float)duration / 1000 << " s" << endl;
}

int main() {
    cout << "Starting benchmarks..." << endl;

    benchmarkMatch3000Orders();
    benchmarkMatchMillionOrders();
    benchmarkOrdersProcessedIn5Seconds();
    benchmarkOrdersProcessedIn1Second();
    benchmarkOrdersProcessedIn1Millisecond();
    benchmarkAddSingleOrder();
    benchmarkCancelSingleOrder();
    benchmarkMixedOrderMatching(3000);

    cout << "Benchmarks completed." << endl;
    return 0;
}
