#include "types.hpp"

// template to build inbound and outbound buffers
// - inbound buffer, shows ever raw order coming from the networok
// - outbound buffer, shows every internal trade 
template<typename T, size_t Size>
class RingBuffer{
  // ensures preallocated memory 64 byte cache line
  static_assert((Size & (Size -1)) == 0, "Size must be a power of 2 for fast masking. Change size of buffer!");

  public: 
    RingBuffer() {
      data = new T[Size];
      
    }

    ~RingBuffer() {
      delete[] data; 
    }

    bool

  private:
    T* data; 

};