#pragma once

#include <atomic>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string_view>
#include <thread>
#include <utility>

enum class LogLevel : int { DEBUG = 0, INFO = 1, WARN = 2, ERROR = 3 };

class Logger {
 public:
  static void initializeFromEnv() {
    const char* env_value = std::getenv("HFT_LOG_LEVEL");
    if (env_value == nullptr) {
      return;
    }

    const std::string_view level(env_value);
    if (level == "DEBUG") {
      level_.store(LogLevel::DEBUG, std::memory_order_relaxed);
    } else if (level == "INFO") {
      level_.store(LogLevel::INFO, std::memory_order_relaxed);
    } else if (level == "WARN") {
      level_.store(LogLevel::WARN, std::memory_order_relaxed);
    } else if (level == "ERROR") {
      level_.store(LogLevel::ERROR, std::memory_order_relaxed);
    }
  }

  template <typename... Args>
  static void debug(Args&&... args) {
    log(LogLevel::DEBUG, std::forward<Args>(args)...);
  }

  template <typename... Args>
  static void info(Args&&... args) {
    log(LogLevel::INFO, std::forward<Args>(args)...);
  }

  template <typename... Args>
  static void warn(Args&&... args) {
    log(LogLevel::WARN, std::forward<Args>(args)...);
  }

  template <typename... Args>
  static void error(Args&&... args) {
    log(LogLevel::ERROR, std::forward<Args>(args)...);
  }

 private:
  template <typename... Args>
  static void log(LogLevel level, Args&&... args) {
    if (static_cast<int>(level) < static_cast<int>(level_.load(std::memory_order_relaxed))) {
      return;
    }

    std::ostringstream message;
    (message << ... << std::forward<Args>(args));

    std::lock_guard<std::mutex> lock(mutex_);
    std::cerr << timestamp() << " [" << levelName(level) << "] "
              << "[tid:" << std::this_thread::get_id() << "] "
              << message.str() << '\n';
  }

  static const char* levelName(LogLevel level) {
    switch (level) {
      case LogLevel::DEBUG:
        return "DEBUG";
      case LogLevel::INFO:
        return "INFO";
      case LogLevel::WARN:
        return "WARN";
      case LogLevel::ERROR:
        return "ERROR";
    }

    return "UNKNOWN";
  }

  static std::string timestamp() {
    const auto now = std::chrono::system_clock::now();
    const std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    std::tm local_time{};
    localtime_r(&now_time, &local_time);

    std::ostringstream out;
    out << std::put_time(&local_time, "%H:%M:%S");
    return out.str();
  }

  static inline std::atomic<LogLevel> level_{LogLevel::INFO};
  static inline std::mutex mutex_;
};
