#pragma once

#include "logger.hpp"
#include "ring_buffer.hpp"

#include <cerrno>
#include <chrono>
#include <cctype>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <sys/mman.h>
#include <sys/stat.h>
#include <thread>
#include <unistd.h>

template <size_t QueueSize>
class FeedHandler {
 public:
  explicit FeedHandler(RingBuffer<Order, QueueSize>& inbound_queue)
      : inbound_queue_(inbound_queue) {}

  void readAndProcess(const std::string& filename) const {
    Logger::info("FeedHandler opening input file: ", filename);
    const int fd = ::open(filename.c_str(), O_RDONLY);
    if (fd < 0) {
      Logger::error("Failed to open CSV file: ", filename, " (",
                    std::strerror(errno), ")");
      return;
    }

    struct stat file_stat {};
    if (::fstat(fd, &file_stat) != 0) {
      Logger::error("Failed to stat CSV file: ", filename, " (",
                    std::strerror(errno), ")");
      ::close(fd);
      return;
    }

    const std::size_t file_size = static_cast<std::size_t>(file_stat.st_size);
    std::size_t processed_orders = 0;
    std::size_t skipped_lines = 0;
    const auto start = std::chrono::steady_clock::now();

    if (file_size > 0) {
      void* mapped =
          ::mmap(nullptr, file_size, PROT_READ, MAP_PRIVATE, fd, 0);
      if (mapped == MAP_FAILED) {
        Logger::error("Failed to mmap CSV file: ", filename, " (",
                      std::strerror(errno), ")");
        ::close(fd);
        return;
      }

      const char* current = static_cast<const char*>(mapped);
      const char* end = current + file_size;

      while (current < end) {
        if (isLineBreak(*current)) {
          consumeLineEnding(current, end);
          continue;
        }

        Order order{};
        if (!parseLine(current, end, order)) {
          ++skipped_lines;
          skipToNextLine(current, end);
          continue;
        }

        pushOrder(order);
        ++processed_orders;

        if (processed_orders % 250000 == 0) {
          Logger::info("FeedHandler enqueued ", processed_orders, " orders so far");
        }
      }

      ::munmap(mapped, file_size);
    }

    ::close(fd);

    const auto end = std::chrono::steady_clock::now();
    const std::chrono::duration<double> elapsed = end - start;
    const double seconds = elapsed.count();
    const double throughput =
        seconds > 0.0 ? static_cast<double>(processed_orders) / seconds : 0.0;

    std::ostringstream summary;
    summary << "Ingested " << processed_orders << " orders in " << seconds
            << " seconds. Throughput: " << throughput << " orders/sec";
    if (skipped_lines > 0) {
      summary << " (" << skipped_lines << " skipped lines)";
    }
    Logger::info(summary.str());
  }

 private:
  void pushOrder(const Order& order) const {
    while (!inbound_queue_.push(order)) {
      std::this_thread::yield();
    }
  }

  static bool parseLine(const char*& current, const char* end, Order& order) {
    uint64_t id = 0;
    Side side{};
    Trading::PRICE price = 0;
    Trading::QUANTITY quantity = 0;
    OrderType type{};
    Trading::SYMBOL_ID symbol_id = 0;
    Trading::TIMESTAMP timestamp = 0;

    if (!parseUnsigned(current, end, id) || !consumeComma(current, end) ||
        !parseSide(current, end, side) || !consumeComma(current, end) ||
        !parsePrice(current, end, price) || !consumeComma(current, end) ||
        !parseUnsigned(current, end, quantity) || !consumeComma(current, end) ||
        !parseType(current, end, type)) {
      return false;
    }

    skipSpaces(current, end);
    if (current != end && !isLineBreak(*current)) {
      if (*current != ',') {
        return false;
      }
      ++current;

      if (!parseUnsigned(current, end, symbol_id)) {
        return false;
      }

      skipSpaces(current, end);
      if (current != end && !isLineBreak(*current)) {
        if (*current != ',') {
          return false;
        }
        ++current;

        if (!parseUnsigned(current, end, timestamp)) {
          return false;
        }
      }
    }

    if (timestamp == 0) {
      timestamp = id;
    }

    skipSpaces(current, end);
    if (current != end && !isLineBreak(*current)) {
      return false;
    }

    consumeLineEnding(current, end);
    order = Order::Create(price, quantity, symbol_id, side, type, id, timestamp);
    return true;
  }

  static bool isLineBreak(char c) {
    return c == '\n' || c == '\r';
  }

  static void skipSpaces(const char*& current, const char* end) {
    while (current < end && (*current == ' ' || *current == '\t')) {
      ++current;
    }
  }

  static void consumeLineEnding(const char*& current, const char* end) {
    if (current < end && *current == '\r') {
      ++current;
    }
    if (current < end && *current == '\n') {
      ++current;
    }
  }

  static void skipToNextLine(const char*& current, const char* end) {
    while (current < end && !isLineBreak(*current)) {
      ++current;
    }
    consumeLineEnding(current, end);
  }

  static bool consumeComma(const char*& current, const char* end) {
    skipSpaces(current, end);
    if (current == end || *current != ',') {
      return false;
    }
    ++current;
    return true;
  }

  template <typename UInt>
  static bool parseUnsigned(const char*& current, const char* end, UInt& value) {
    static_assert(std::is_unsigned_v<UInt>, "UInt must be unsigned");

    skipSpaces(current, end);
    if (current == end ||
        !std::isdigit(static_cast<unsigned char>(*current))) {
      return false;
    }

    uint64_t parsed = 0;
    while (current < end &&
           std::isdigit(static_cast<unsigned char>(*current)) != 0) {
      parsed = parsed * 10 + static_cast<unsigned>(*current - '0');
      ++current;
    }

    skipSpaces(current, end);
    if (parsed > std::numeric_limits<UInt>::max()) {
      return false;
    }

    value = static_cast<UInt>(parsed);
    return true;
  }

  static bool parsePrice(const char*& current, const char* end,
                         Trading::PRICE& price) {
    skipSpaces(current, end);
    if (current == end) {
      return false;
    }

    bool negative = false;
    if (*current == '+' || *current == '-') {
      negative = (*current == '-');
      ++current;
    }

    if (current == end ||
        !std::isdigit(static_cast<unsigned char>(*current))) {
      return false;
    }

    uint64_t whole = 0;
    while (current < end &&
           std::isdigit(static_cast<unsigned char>(*current)) != 0) {
      whole = whole * 10 + static_cast<unsigned>(*current - '0');
      ++current;
    }

    if (current < end && *current == '.') {
      ++current;

      uint64_t fractional = 0;
      int digits = 0;
      while (current < end &&
             std::isdigit(static_cast<unsigned char>(*current)) != 0) {
        if (digits < 8) {
          fractional = fractional * 10 + static_cast<unsigned>(*current - '0');
          ++digits;
        }
        ++current;
      }

      while (digits < 8) {
        fractional *= 10;
        ++digits;
      }

      const uint64_t scaled =
          whole * static_cast<uint64_t>(Trading::SCALE) + fractional;
      price = negative ? -static_cast<Trading::PRICE>(scaled)
                       : static_cast<Trading::PRICE>(scaled);
    } else {
      price = negative ? -static_cast<Trading::PRICE>(whole)
                       : static_cast<Trading::PRICE>(whole);
    }

    skipSpaces(current, end);
    return true;
  }

  static bool parseSide(const char*& current, const char* end, Side& side) {
    skipSpaces(current, end);
    if (current == end) {
      return false;
    }

    const char side_char =
        static_cast<char>(std::toupper(static_cast<unsigned char>(*current)));
    ++current;
    skipSpaces(current, end);

    if (side_char == 'B') {
      side = Side::BUY;
      return true;
    }
    if (side_char == 'S') {
      side = Side::SELL;
      return true;
    }

    return false;
  }

  static bool parseType(const char*& current, const char* end,
                        OrderType& type) {
    skipSpaces(current, end);
    if (current == end) {
      return false;
    }

    const char type_char =
        static_cast<char>(std::toupper(static_cast<unsigned char>(*current)));
    ++current;
    skipSpaces(current, end);

    if (type_char == 'L') {
      type = OrderType::LIMIT;
      return true;
    }
    if (type_char == 'M') {
      type = OrderType::MARKET;
      return true;
    }

    return false;
  }

  RingBuffer<Order, QueueSize>& inbound_queue_;
};
