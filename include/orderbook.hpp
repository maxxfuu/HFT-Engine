#ifndef ORDERBOOK_HPP
#define ORDERBOOK_HPP

#include "types.hpp"
#include <map>
#include <unordered_map>
#include <list> 
#include <deque>

// holds all orders within the price level
struct PriceLevel {
  Trading::PRICE price;
  Trading::QUANTITY quantity;
  std::list<Order> orders;
};

class OrderBook {
  public:
    void handleOrder(Order& order);

    void cancelOrder(int64_t id);

  private:
    std::map<Trading::PRICE, PriceLevel, std::greater<Trading::PRICE>> bids;
    std::map<Trading::PRICE, PriceLevel> ask;

    std::unordered_map<uint64_t, std::list<Order>::iterator> order_lookup;

    // top of book 
    PriceLevel* best_bid = nullptr;
    PriceLevel* best_ask = nullptr;
};

#endif