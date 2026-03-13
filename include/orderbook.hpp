#ifndef ORDERBOOK_HPP
#define ORDERBOOK_HPP

#include "types.hpp"
#include <map>
#include <deque>

class OrderBook {
  public:
    OrderBook();
    ~OrderBook();

    void add_order(Order& order);
    void cancel_order(Order& order);
    void match_order(Order& order);

  private:
    std::map<int64_t, std::deque<Order>> bids;
    std::map<int64_t, std::deque<Order>> asks;
};

#endif