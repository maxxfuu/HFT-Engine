#pragma once

#include "types.hpp"
#include "orderbook.hpp"
#include "ring_buffer.hpp" 

#include <array>
#include <atomic>
#include <thread>
#include <vector>

namespace Trading {
  // Flat arrays provide O(1) access and are much more cache-friendly than maps.
  constexpr size_t MAX_SYMBOLS = 10'000;
  constexpr size_t INBOUND_QUEUE_SIZE = 65536; 
  constexpr size_t OUTBOUND_QUEUE_SIZE = 65536;
}

class MatchingEngine {
public:
  MatchingEngine(RingBuffer<Order, Trading::INBOUND_QUEUE_SIZE>& inbound, RingBuffer<Trade, Trading::OUTBOUND_QUEUE_SIZE>& outbound);
  ~MatchingEngine();

  // delete copy/move constructors to prevent accidental duplication
  MatchingEngine(const MatchingEngine&) = delete;
  MatchingEngine& operator=(const MatchingEngine&) = delete;

  void start();
  void stop();

private:
  // this will poll the inbound queue, route orders, and manage the order books.
  void run();

  std::array<OrderBook, Trading::MAX_SYMBOLS> order_books_;

  RingBuffer<Order, Trading::INBOUND_QUEUE_SIZE>& inbound_queue_;
  RingBuffer<Trade, Trading::OUTBOUND_QUEUE_SIZE>& outbound_queue_;

  std::atomic<bool> running_{false};
  std::thread worker_thread_;
};