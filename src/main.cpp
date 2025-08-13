#include "Orderbook.h"
#include "Order.h"
#include "OrderTypes.h"
#include "OrderGenerator.h"

#include "types.h"
#include <iostream>
#include <chrono>
#include "httplib.h"

using namespace std;
using namespace std::chrono;

int main() {
    // Orderbook book;
    // OrderGenerator generator;
    // OrderID id = 0;
    // generator.setType(OrderType::MARKET);
    // auto orders = generator.generateOrders(1000000, id);

    // cout << book.getMidPrice() << endl;

    // auto start = high_resolution_clock::now();
    // for (auto &order : orders) {
    //     book.addOrder(order);
    // }
    // auto end = high_resolution_clock::now();
    // auto duration = duration_cast<milliseconds>(end - start).count();

    // cout << book.getMidPrice() << endl;

    // cout << "Time taken to match 1,000,000 orders: " << (float)duration / 1000 << " s" << endl;

    Orderbook orderBook;
    httplib::Server svr;

    svr.Post("/add_order", [&](const httplib::Request& req, httplib::Response& res) {
        
    });

    // Route for removing an order
    svr.Post("/remove_order", [&](const httplib::Request& req, httplib::Response& res) {
        
    });

    std::cout << "Server running on port 8080" << std::endl;
    if (svr.listen("0.0.0.0", 8080)) {
        std::cout << "Started server" << std::endl;
    } else {
        std::cerr << "Failed to start server" << std::endl;
        return -1;
    }

    return 0;
}