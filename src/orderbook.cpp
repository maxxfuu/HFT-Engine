#include "orderbook.hpp"

void OrderBook::handleOrder(Order& order) {
  // for simplicity, assume all orders handled by LOB are all Limit type 
  if (order.type == OrderType::LIMIT) {
    addOrder(order);
  }
}

void OrderBook::addOrder(Order& order) {
  if (order.side == Side::BUY) {

    auto [level_itr, inserted] = bids.try_emplace(order.price, order.price);
    auto& level = level_itr->second;

    level.orders.push_back(order);
    level.quantity += order.quantity;

    auto order_itr = level.orders.end();
    --order_itr;
    order_lookup[order.id] = OrderLocation{order.price, order.side, order_itr};
    best_bid = &bids.begin()->second;
  } else {
    auto [level_itr, inserted] = asks.try_emplace(order.price, order.price);
    auto& level = level_itr->second;

    level.orders.push_back(order);
    level.quantity += order.quantity;

    auto order_itr = level.orders.end(); // points past the last node 
    --order_itr; // iterates back 1 step on to the last node 
    order_lookup[order.id] = OrderLocation{order.price, order.side, order_itr};
    best_ask = &asks.begin()->second;
  }
}

void OrderBook::cancelOrder(int64_t id) {
  // pointer to OrderLocation struct
  auto order_itr = order_lookup.find(id);
  if (order_itr == order_lookup.end()) return;

  OrderLocation& loc = order_itr->second;
  if (loc.side == Side::BUY) {
    auto level_itr = bids.find(loc.price);
    if (level_itr == bids.end()) return;

    auto& level = level_itr->second;
    const auto quantity = loc.itr->quantity;

    level.orders.erase(loc.itr);
    level.quantity -= quantity;

    if (level.orders.empty()) {
      bids.erase(level_itr);
    }

    best_bid = bids.empty() ? nullptr : &bids.begin()->second;
  } else {
    auto level_itr = asks.find(loc.price);
    if (level_itr == asks.end()) return;

    auto& level = level_itr->second;
    const auto quantity = loc.itr->quantity;

    level.orders.erase(loc.itr);
    level.quantity -= quantity;

    if (level.orders.empty()) {
      asks.erase(level_itr);
    }

    best_ask = asks.empty() ? nullptr : &asks.begin()->second;
  }

  order_lookup.erase(order_itr);
}

