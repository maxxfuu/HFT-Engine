# Order Book and Trading Engine Plan 

This project is a high-performance C++17 trading engine. The HFT Engine comprises of a Limit Order Book, the Feed Handler, Ring Buffers, and the Matching Engine. 

To put it simply, it is a price-time priority limit order book matching engine supporting limit orders, market orders, and cancelations. 

When designing this HFT Engine, the decisions made were considered through the lense of writing safe and performant C++. This includes using atomics instead of mutexs. Thinking about how to leverage the CPU and OS such as utilizing the concept of hardware prefetcher, efficently manage memory, using implement data structures (ring buffer) and patterns (producer & consumer). 