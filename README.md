# HFT-System
a high-frequency trading engine  designed to simulate ingesting market data and execute trades with microsecond latency. Users interact through the web dashboard whcih displays real-time order book "ladder" and equity curve. Connected via WebSockets to a C++ 17 core engine


## Step 1: Define Your Data Types (types.hpp)
You need "Plain Old Data" (POD) structures that are cache-friendly and easily moved.

The Enums: Create an enum class for Side (Buy/Sell) and OrderType (Limit/Market).

The Structs: Define a Tick struct (Symbol, Price, Quantity) and an Order struct.

Concepts: Use std::variant to wrap these into a single Event type for your queue.

Reference: Tour of C++ (3rd Ed) Section 2.4 (Enums) and Section 2.5 (Unions/Variants).

## Step 2: Build the Lock-Free Pipeline (ring_buffer.hpp)
Standard queues use mutexes, which put threads to "sleep" while waiting. You must implement a Single Producer Single Consumer (SPSC) Ring Buffer to keep threads awake and spinning.

General Structure: A fixed-size std::array of your Event objects.

Implementation Logic: Use two std::atomic<size_t> variables: head (where the producer writes) and tail (where the consumer reads).

Optimization: Apply alignas(64) to your atomic indices to prevent False Sharing—ensuring the producer and consumer aren't fighting over the same CPU cache line.

Reference: Concurrency in Action Chapter 5 (Atomics) and Tour of C++ Section 1.9 (Hardware Mapping).

## Step 3: Implement the Order Book (orderbook.hpp/cpp)
This is the "brain" of the engine where you track market liquidity.

General Structure: Two collections (one for Bids, one for Asks).

Implementation Logic: Use std::map<Price, Quantity> or a sorted std::vector for the MVP.

RAII & Ownership: The OrderBook should be managed by a std::unique_ptr in the Strategy thread, ensuring only that thread can modify the memory.

Reference: Tour of C++ Section 6.3 (Resource Management/RAII) and Section 15.2 (Smart Pointers).

## Step 4: Thread Orchestration (engine.hpp/cpp)
You must bind your logic to specific CPU cores to avoid "context switching" latency.

General Structure: A class that spawns and manages three std::thread objects: Inbound, Strategy, and Outbound.

The Loop Logic: Each thread runs a while(running) loop.

Inbound: Listens to a UDP socket and pushes to RingBuffer A.

Strategy: Pops from RingBuffer A, updates the OrderBook, and pushes to RingBuffer B.

Outbound: Pops from RingBuffer B and logs the execution.

Move Semantics: Ensure the Strategy thread uses std::move when passing orders to the Outbound thread to avoid expensive copies.

Reference: Tour of C++ Section 18.2 (Threads) and Effective C++ Item 23 (Move Semantics).

## Step 5: Entry Point (main.cpp)
Logic: Initialize your atomic flags, spawn the engine, and set up a signal handler (like SIGINT) to shut down the threads gracefully using RAII.

Reference: Tour of C++ Section 1.5 (Scope and Lifetime).
