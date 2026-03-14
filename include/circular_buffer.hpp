#include "types.hpp"

// template to build inbound and outbound buffers
// - inbound buffer, shows ever raw order coming from the networok
// - outbound buffer, shows every internal trade 
template<typename T, size_t Size>
class RingBuffer{
  // ensures preallocated memory 64 byte cache line
  static_assert((Size & (Size -1)) == 0, "Size must be a power of 2 for fast masking. Change size of buffer!");
  
  public: 
    RingBuffer() { data = new T[Size](); }
    ~RingBuffer() { delete[] data; }

    // producer path
    bool push() {
      alignas(64) std::atomic<uint64_t> write_idx = 0;
      alignas(64) std::atomic<uint64_t> read_idx = 0; 
      
      if (head - tail >= Size) return false; // buffer full, lapping protection
  
      data[head & mask] = item;
  
      write_idx.store(head + 1, std::memory_order_release);
      return true;
    } 
    
    // consumer path 
    bool delete() {
      uint64_t tail = read_idx.load(std::memory_order_relaxed);
      uint64_t head = write_idx.load(std::memory_order_acquire);

      if (tail == head) return false; // checks for empty

      output = data[tail & mask];
      read_index.store(tail + 1, std::memory_order_release);
      return true;
    }

  private:
    T* data; 

    alignas(64) std::atomic<uint64_t> write_idx{0};
    alignas(64) std::atomic<uint64_t> read_idx{0};
};