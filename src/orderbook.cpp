#include "orderbook.hpp"

OrderBook::OrderBook() : bids(), asks() {}

OrderBook::~OrderBook() {}

void OrderBook::add_order(Order& order) {
  if (order.type == OrderType::MARKET) {
    // inside match immediate logic, checks side 
    match_immediate(order);
    return;
  }

  if (order.side == Side::BUY) {
    // move the order into the deque to prevent copying
    bids[order.price].push_back(std::move(order));
  } else {
    asks[order.price].push_back(std::move(order));
  }
}

