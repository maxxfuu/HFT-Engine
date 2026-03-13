#include "orderbook.hpp"

void OrderBook::handleOrder(Order& order) {
  // for simplicity, assume all orders handled by LOB are all Limit type 
  if (order.type == OrderType::LIMIT) {
    addOrder(order);
  }
}

void OrderBook::addOrder(Order& order) {
  if (order.side == Side::BUY) {
    auto [level_it, inserted] = bids.try_emplace(order.price, order.price);
    auto& level = level_it->second;

    level.orders.push_back(order);
    level.quantity += order.quantity;

    auto order_it = level.orders.end();
    --order_it;
    order_lookup[order.id] = OrderLocation{order.price, order.side, order_it};
    best_bid = &bids.begin()->second;
  } else {
    auto [level_it, inserted] = asks.try_emplace(order.price, order.price);
    auto& level = level_it->second;

    level.orders.push_back(order);
    level.quantity += order.quantity;

    auto order_it = level.orders.end();
    --order_it;
    order_lookup[order.id] = OrderLocation{order.price, order.side, order_it};
    best_ask = &asks.begin()->second;
  }
}

void OrderBook::cancelOrder(int64_t id) {
  
}

