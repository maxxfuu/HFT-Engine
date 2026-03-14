#include "orderbook.hpp"
#include "feed_handler.hpp"

int main() {
    OrderBook book;
    FeedHandler handler;

    std::cout << "Starting 1,000,000 order blast..." << std::endl;
    handler.readAndProcess("market_data.csv", book);
    
    // Check state after the blast
    std::cout << "Final Best Bid: " << book.getBestBid() << std::endl;
    std::cout << "Final Best Ask: " << book.getBestAsk() << std::endl;

    return 0;
}