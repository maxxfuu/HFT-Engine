#include "matching_engine.hpp"

#include <thread>

MatchingEngine::MatchingEngine(
    RingBuffer<Order, Trading::INBOUND_QUEUE_SIZE>& inbound,
    RingBuffer<Trade, Trading::OUTBOUND_QUEUE_SIZE>& outbound)
    : inbound_queue_(inbound), outbound_queue_(outbound) {}

MatchingEngine::~MatchingEngine() {
  stop();
}

void MatchingEngine::start() {
  bool expected = false;
  if (!running_.compare_exchange_strong(expected, true,
                                        std::memory_order_acq_rel)) {
    return;
  }

  worker_thread_ = std::thread(&MatchingEngine::run, this);
}

void MatchingEngine::stop() {
  bool expected = true;
  if (!running_.compare_exchange_strong(expected, false,
                                        std::memory_order_acq_rel)) {
    if (worker_thread_.joinable()) {
      worker_thread_.join();
    }
    return;
  }

  if (worker_thread_.joinable()) {
    worker_thread_.join();
  }
}

void MatchingEngine::run() {
  auto process_order = [this](const Order& order) {
    if (order.symbol_id >= Trading::MAX_SYMBOLS) {
      return;
    }

    std::vector<Trade> trades = order_books_[order.symbol_id].handleOrder(order);
    for (const Trade& trade : trades) {
      while (!outbound_queue_.push(trade)) {
        std::this_thread::yield();
      }
    }
  };

  Order inbound_order{};
  while (running_.load(std::memory_order_acquire)) {
    if (!inbound_queue_.pop(inbound_order)) {
      std::this_thread::yield();
      continue;
    }

    process_order(inbound_order);
  }

  while (inbound_queue_.pop(inbound_order)) {
    process_order(inbound_order);
  }
}
