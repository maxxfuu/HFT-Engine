#pragma once
#include <cstdint>
#include <variant>

enum class Side {
  BUY,
  SELL,
};

enum class OrderType {
  LIMIT,
  MARKET,
};

struct Order {
  uint64_t id;
  double price;
  uint32_t quantity;
  Side side;
  OrderType type;
};

