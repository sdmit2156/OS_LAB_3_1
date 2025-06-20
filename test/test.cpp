#include "marker.h"
#include "catch_amalgamated.hpp"
#include <vector>
#include <atomic>
#include <condition_variable>
#include <memory>
#include <thread>
#include <chrono>

TEST_CASE("Marker thread marks array correctly", "[marker]") {
    const int size = 10;
    std::vector<int> arr(size, 0);
    std::mutex arrayLock, consoleLock;
    std::vector<int> markedIndices;

    std::condition_variable startCV;
    std::condition_variable pauseCV;
    std::atomic<bool> startSignal(false);

    auto stopSignal = std::make_shared<std::atomic<bool>>(false);
    auto pausedSignal = std::make_shared<std::atomic<bool>>(false);
    auto finishedSignal = std::make_shared<std::atomic<bool>>(false);

    MarkerParams params = {
        1,
        arr.data(),
        size,
        arrayLock,
        consoleLock,
        markedIndices,
        startCV,
        pauseCV,
        startSignal,
        stopSignal,
        pausedSignal,
        finishedSignal
    };

    std::thread thread(MarkerThread, &params);

    {
        std::lock_guard<std::mutex> lock(arrayLock);
        startSignal = true;
    }
    startCV.notify_all();

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    bool marked = false;
    {
        std::lock_guard<std::mutex> lock(arrayLock);
        for (int val : arr) {
            if (val == 1) {
                marked = true;
                break;
            }
        }
    }
    REQUIRE(marked == true);

    stopSignal->store(true);
    pausedSignal->store(false);
    pauseCV.notify_all();

    thread.join();

    bool cleared = true;
    {
        std::lock_guard<std::mutex> lock(arrayLock);
        for (int val : arr) {
            if (val != 0) {
                cleared = false;
                break;
            }
        }
    }
    REQUIRE(cleared == true);
}


TEST_CASE("Marker thread handles termination correctly", "[marker]") {
    const int size = 10;
    std::vector<int> arr(size, 0);
    std::mutex arrayLock, consoleLock;
    std::vector<int> markedIndices;

    std::condition_variable startCV;
    std::condition_variable pauseCV;
    std::atomic<bool> startSignal(false);

    auto stopSignal = std::make_shared<std::atomic<bool>>(false);
    auto pausedSignal = std::make_shared<std::atomic<bool>>(false);
    auto finishedSignal = std::make_shared<std::atomic<bool>>(false);

    MarkerParams params = {
        1,
        arr.data(),
        size,
        arrayLock,
        consoleLock,
        markedIndices,
        startCV,
        pauseCV,
        startSignal,
        stopSignal,
        pausedSignal,
        finishedSignal
    };

    std::thread thread(MarkerThread, &params);

    {
        std::lock_guard<std::mutex> lock(arrayLock);
        startSignal = true;
    }
    startCV.notify_all();

    stopSignal->store(true);
    pausedSignal->store(false);
    pauseCV.notify_all();

    thread.join();

    REQUIRE(finishedSignal->load() == true);
}
