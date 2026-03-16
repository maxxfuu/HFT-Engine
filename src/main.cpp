#include "feed_handler.hpp"
#include "matching_engine.hpp"

#include <atomic>
#include <cstddef>
#include <iostream>
#include <thread>

int main() {
    RingBuffer<Order, Trading::INBOUND_QUEUE_SIZE> inbound_queue;
    RingBuffer<Trade, Trading::OUTBOUND_QUEUE_SIZE> outbound_queue;
    FeedHandler<Trading::INBOUND_QUEUE_SIZE> handler(inbound_queue);
    MatchingEngine engine(inbound_queue, outbound_queue);
    std::atomic<bool> engine_finished{false};
    std::atomic<std::size_t> emitted_trades{0};

    std::thread trade_logger([&]() {
        Trade trade{};
        std::size_t local_emitted_trades = 0;

        while (true) {
            if (outbound_queue.pop(trade)) {
                ++local_emitted_trades;
                continue;
            }

            if (engine_finished.load(std::memory_order_acquire)) {
                break;
            }

            std::this_thread::yield();
        }

        emitted_trades.store(local_emitted_trades, std::memory_order_release);
    });

    std::cout << "Starting order blast..." << std::endl;
    engine.start();
    handler.readAndProcess("market_data.csv");
    engine.stop();
    engine_finished.store(true, std::memory_order_release);
    trade_logger.join();

    std::cout << "Summary: emitted "
              << emitted_trades.load(std::memory_order_acquire)
              << " trades." << std::endl;

    return 0;
}