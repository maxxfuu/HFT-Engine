#include "orderbook.hpp"

#include <algorithm>

std::vector<Trade> OrderBook::handleOrder(const Order& order) {
  if (order.type != OrderType::LIMIT) {
    return {};
  }

  return handleLimitOrder(order);
}

std::vector<Trade> OrderBook::handleLimitOrder(const Order& order) {
  Order working_order = order;
  std::vector<Trade> trades;

  if (working_order.side == Side::BUY) {
    matchBuyOrder(working_order, trades);
  } else {
    matchSellOrder(working_order, trades);
  }

  if (working_order.quantity > 0) {
    addOrder(working_order);
  }

  return trades;
}

void OrderBook::matchBuyOrder(Order& order, std::vector<Trade>& trades) {
  while (order.quantity > 0 && !asks.empty()) {
    auto level_itr = asks.begin();
    if (level_itr->first > order.price) {
      break;
    }

    auto& level = level_itr->second;
    while (order.quantity > 0 && !level.orders.empty()) {
      Order& resting = level.orders.front();
      const Trading::QUANTITY executed_quantity =
          std::min(order.quantity, resting.quantity);

      trades.push_back(Trade{next_trade_id_++, order.id, resting.id,
                             resting.price, executed_quantity, order.timestamp});

      order.quantity -= executed_quantity;
      resting.quantity -= executed_quantity;
      level.quantity -= executed_quantity;

      if (resting.quantity == 0) {
        order_lookup.erase(resting.id);
        level.orders.pop_front();
      }
    }

    if (level.orders.empty()) {
      asks.erase(level_itr);
    }
  }

  updateBestAsk();
}

void OrderBook::matchSellOrder(Order& order, std::vector<Trade>& trades) {
  while (order.quantity > 0 && !bids.empty()) {
    auto level_itr = bids.begin();
    if (level_itr->first < order.price) {
      break;
    }

    auto& level = level_itr->second;
    while (order.quantity > 0 && !level.orders.empty()) {
      Order& resting = level.orders.front();
      const Trading::QUANTITY executed_quantity =
          std::min(order.quantity, resting.quantity);

      trades.push_back(Trade{next_trade_id_++, resting.id, order.id,
                             resting.price, executed_quantity, order.timestamp});

      order.quantity -= executed_quantity;
      resting.quantity -= executed_quantity;
      level.quantity -= executed_quantity;

      if (resting.quantity == 0) {
        order_lookup.erase(resting.id);
        level.orders.pop_front();
      }
    }

    if (level.orders.empty()) {
      bids.erase(level_itr);
    }
  }

  updateBestBid();
}

void OrderBook::addOrder(const Order& order) {
  if (order.side == Side::BUY) {

    auto [level_itr, inserted] = bids.try_emplace(order.price, order.price);
    auto& level = level_itr->second;

    level.orders.push_back(order);
    level.quantity += order.quantity;

    auto order_itr = level.orders.end();
    --order_itr;
    order_lookup[order.id] = OrderLocation{order.price, order.side, order_itr};
    updateBestBid();
  } else {
    auto [level_itr, inserted] = asks.try_emplace(order.price, order.price);
    auto& level = level_itr->second;

    level.orders.push_back(order);
    level.quantity += order.quantity;

    auto order_itr = level.orders.end(); // points past the last node 
    --order_itr; // iterates back 1 step on to the last node 
    order_lookup[order.id] = OrderLocation{order.price, order.side, order_itr};
    updateBestAsk();
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

    updateBestBid();
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

    updateBestAsk();
  }

  order_lookup.erase(order_itr);
}

void OrderBook::updateBestBid() {
  best_bid = bids.empty() ? nullptr : &bids.begin()->second;
}

void OrderBook::updateBestAsk() {
  best_ask = asks.empty() ? nullptr : &asks.begin()->second;
}

