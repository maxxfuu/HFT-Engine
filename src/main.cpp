#include "feed_handler.hpp"
#include "logger.hpp"
#include "matching_engine.hpp"

#include <atomic>
#include <cstddef>
#include <thread>

int main() {
    Logger::initializeFromEnv();

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

                if (local_emitted_trades % 250000 == 0) {
                    Logger::info("Trade consumer drained ", local_emitted_trades,
                                 " trades so far");
                }
                continue;
            }

            if (engine_finished.load(std::memory_order_acquire)) {
                break;
            }

            std::this_thread::yield();
        }

        emitted_trades.store(local_emitted_trades, std::memory_order_release);
        Logger::info("Trade consumer exiting after draining ",
                     local_emitted_trades, " trades");
    });

    Logger::info("Starting order blast");
    engine.start();
    handler.readAndProcess("market_data.csv");
    engine.stop();
    engine_finished.store(true, std::memory_order_release);
    trade_logger.join();

    Logger::info("Run complete. Emitted ", emitted_trades.load(std::memory_order_acquire),
                 " trades.");

    return 0;
}