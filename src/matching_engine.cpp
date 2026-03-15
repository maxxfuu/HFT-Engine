#include "matching_engine.hpp"

#include "logger.hpp"

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
    Logger::warn("MatchingEngine start() called while already running");
    return;
  }

  Logger::info("MatchingEngine starting worker thread");
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
    Logger::info("MatchingEngine stopping worker thread");
    worker_thread_.join();
  }
}

void MatchingEngine::run() {
  Logger::info("MatchingEngine event loop started");
  std::size_t processed_orders = 0;
  std::size_t emitted_trades = 0;
  std::size_t invalid_symbol_orders = 0;

  auto process_order_with_stats = [this, &processed_orders, &emitted_trades,
                                   &invalid_symbol_orders](const Order& order) {
    if (order.symbol_id >= Trading::MAX_SYMBOLS) {
      ++invalid_symbol_orders;
      if (invalid_symbol_orders <= 5) {
        Logger::warn("Skipping order ", order.id, " with invalid symbol_id ",
                     order.symbol_id);
      }
      return;
    }

    std::vector<Trade> trades = order_books_[order.symbol_id].handleOrder(order);
    ++processed_orders;
    emitted_trades += trades.size();

    if (processed_orders % 250000 == 0) {
      Logger::info("MatchingEngine processed ", processed_orders,
                   " inbound orders and emitted ", emitted_trades, " trades");
    }

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

    process_order_with_stats(inbound_order);
  }

  while (inbound_queue_.pop(inbound_order)) {
    process_order_with_stats(inbound_order);
  }

  Logger::info("MatchingEngine event loop exiting after processing ",
               processed_orders, " orders, emitting ", emitted_trades,
               " trades, invalid symbols: ", invalid_symbol_orders);
}
