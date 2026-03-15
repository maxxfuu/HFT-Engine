#pragma once

#include "types.hpp"
#include <atomic>
#include <cstddef>

// Template to build inbound and outbound buffers.
// - inbound buffer stores raw orders coming from the network
// - outbound buffer stores internal trades

template<typename T, size_t Size>
class RingBuffer {
  static_assert((Size & (Size - 1)) == 0, "Size must be a power of 2 for fast masking. Change size of buffer!");

public:
  RingBuffer() : data(new T[Size]()) {}
  ~RingBuffer() { delete[] data; }

  // Producer path.
  bool push(const T& item) {
    const uint64_t head = write_idx.load(std::memory_order_relaxed);
    const uint64_t tail = read_idx.load(std::memory_order_acquire);

    if (head - tail >= Size) return false; // Buffer full, lapping protection.

    data[head & kMask] = item;
    write_idx.store(head + 1, std::memory_order_release);
    return true;
  }

  // Consumer path.
  bool pop(T& output) {
    const uint64_t tail = read_idx.load(std::memory_order_relaxed);
    const uint64_t head = write_idx.load(std::memory_order_acquire);

    if (tail == head) return false; // Buffer empty.

    output = data[tail & kMask];
    read_idx.store(tail + 1, std::memory_order_release);
    return true;
  }

private:
  static constexpr size_t kMask = Size - 1;

  T* data;

  alignas(64) std::atomic<uint64_t> write_idx{0};
  alignas(64) std::atomic<uint64_t> read_idx{0};
};