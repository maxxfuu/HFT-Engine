#pragma once
#include <cstdint>
#include <variant>

namespace Trading {
  using PRICE = int64_t;
  using TIMESTAMP = uint64_t;
  using QUANTITY = uint32_t;
  using SYMBOL_ID = uint32_t;

  constexpr PRICE SCALE = 100'000'000;

  // Used when coverting a double price to a fixed price
  inline PRICE ToFixed(double price) {
    return static_cast<PRICE>(price * SCALE + 0.5);
  }
}

enum class Side : uint8_t { BUY, SELL };
enum class OrderType : uint8_t { LIMIT, MARKET };

struct alignas(64) Order {
  // 8 bytes
  uint64_t id;
  Trading::PRICE price;
  Trading::TIMESTAMP timestamp;

  // 4 bytes
  Trading::QUANTITY quantity;
  Trading::SYMBOL_ID symbol_id;

  // 1 byte
  Side side;
  OrderType type;
  
  // Automated padding to ensure 64 bytes cache line alignment
  char padding[64 - sizeof(uint64_t) - sizeof(Trading::PRICE) - sizeof(Trading::TIMESTAMP) - sizeof(Trading::QUANTITY) - sizeof(Trading::SYMBOL_ID) - sizeof(Side) - sizeof(OrderType)];

  static Order Create(Trading::PRICE price, Trading::QUANTITY quantity, Trading::SYMBOL_ID symbol_id, Side side, OrderType type, uint64_t id, Trading::TIMESTAMP timestamp) {
    // perform value initalization to prevent garbage values left over in RAM 
    Order order{};

    order.id = id;
    order.price = price;
    order.timestamp = timestamp;
    order.quantity = quantity;
    order.symbol_id = symbol_id;
    order.side = side;
    order.type = type;
    return order;
    };
};

struct alignas(64) Trade {
  uint64_t trade_id;
  uint64_t buyer_order_id;
  uint64_t seller_order_id;
  Trading::PRICE price;
  Trading::QUANTITY quantity;
  Trading::TIMESTAMP timestamp_ns;
};