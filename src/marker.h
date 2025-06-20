#pragma once

#include <vector>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <memory>

struct MarkerParams {
    int id;
    int* array;
    int size;

    std::mutex& arrayMutex;
    std::mutex& coutMutex;
    std::vector<int>& markedIndices;

    std::condition_variable& startCV;
    std::condition_variable& pauseCV;

    std::atomic<bool>& startSignal;
    std::shared_ptr<std::atomic<bool>> stopSignal;
    std::shared_ptr<std::atomic<bool>> pausedSignal;
    std::shared_ptr<std::atomic<bool>> finishedSignal;
};

void MarkerThread(MarkerParams* params);
