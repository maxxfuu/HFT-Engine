#include "orderbook.hpp"
#include <iostream>
#include <iomanip>

// Helper to print the current state of the book
void printTop(OrderBook& book) {
    std::cout << "---------------------------------" << std::endl;
    std::cout << "BEST BID: " << std::fixed << std::setprecision(2) << book.getBestBid() << std::endl;
    std::cout << "BEST ASK: " << std::fixed << std::setprecision(2) << book.getBestAsk() << std::endl;
    std::cout << "---------------------------------" << std::endl;
}

int main() {
    OrderBook book;
    uint64_t next_id = 1;

    std::cout << "Day 1: Placing initial orders..." << std::endl;

    // 1. Place a Buy Order (The Bid)
    // Price: 100.50, Qty: 10, ID: 1
    Order bid = Order::Create(10050, 10, 1, Side::BUY, OrderType::LIMIT, next_id++, 123456789);
    book.handleOrder(bid);
    std::cout << "Placed BID at 100.50" << std::endl;

    // 2. Place a higher Buy Order (Should become new Best Bid)
    Order higher_bid = Order::Create(10200, 5, 1, Side::BUY, OrderType::LIMIT, next_id++, 123456790);
    book.handleOrder(higher_bid);
    std::cout << "Placed BID at 102.00" << std::endl;

    // 3. Place a Sell Order (The Ask)
    Order ask = Order::Create(10500, 20, 1, Side::SELL, OrderType::LIMIT, next_id++, 123456791);
    book.handleOrder(ask);
    std::cout << "Placed ASK at 105.00" << std::endl;

    // Check the results
    printTop(book);

    // 4. Cancel the best bid (102.00)
    std::cout << "Cancelling Order ID 2 (The 102.00 bid)..." << std::endl;
    book.cancelOrder(2);

    // Verify it reverted to the 100.50 bid
    printTop(book);

    return 0;
}