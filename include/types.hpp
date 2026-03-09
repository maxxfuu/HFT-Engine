#pragma once
#include <cstdint>
#include <variant>

enum class Side {
  BUY,
  SELL,
  NONE,
};

enum class OrderType {
  LIMIT,
  MARKET,
  CANCEL,
};

struct Tick {
  char symbol[8];
  double price;
  uint32_t quantity;
};

struct Order {
  char symbol[8];
  Side side;
  OrderType type;
  double price;
  uint32_t quantity;
};

using Event = std::variant<Tick, Order>;
