#pragma once

#include "types.hpp"

#include <cstdint>
#include <functional>
#include <list> 
#include <map>
#include <vector>
#include <unordered_map>

// holds all orders within the price level
struct PriceLevel {
  Trading::PRICE price;
  Trading::QUANTITY quantity;
  std::list<Order> orders;
  PriceLevel(Trading::PRICE p) : price(p), quantity(0){}
};

class OrderBook {
  public:
    std::vector<Trade> handleOrder(const Order& order);
    void cancelOrder(int64_t id);

    Trading::PRICE getBestBid() {
      return bids.empty() ? 0 : bids.begin()->first;
    }

    Trading::PRICE getBestAsk() {
      return asks.empty() ? 0 : asks.begin()->first; 
    }

  private:
    // high to low price level 
    std::map<Trading::PRICE, PriceLevel, std::greater<Trading::PRICE>> bids;
    // low to high price level  
    std::map<Trading::PRICE, PriceLevel> asks;

    // top of book 
    PriceLevel* best_bid = nullptr;
    PriceLevel* best_ask = nullptr;

    // iter holds the location of the node within the PriceLevel Doubly Linkedlist
    struct OrderLocation {
      Trading::PRICE price;
      Side side; 
      std::list<Order>::iterator itr;
    };

    // stores key: id, value: struct Price Side
    std::unordered_map<uint64_t, OrderLocation> order_lookup;

    // internal helper functions after dispatch
    void addOrder(const Order& order);
    std::vector<Trade> handleLimitOrder(const Order& order);
    std::vector<Trade> handleMarketOrder(const Order& order);
    
    void matchBuyLimitOrder(Order& order, std::vector<Trade>& trades);
    void matchSellLimitOrder(Order& order, std::vector<Trade>& trades);

    void matchBuyMarketOrder(Order& order, std::vector<Trade>& trades);
    void matchSellMarketOrder(Order& order, std::vector<Trade>& trades);

    void updateBestBid();
    void updateBestAsk();

    uint64_t next_trade_id_ = 1;

};