#include "marker.h"
#include <iostream>
#include <cstdlib>
#include <thread>
#include <chrono>

void MarkerThread(MarkerParams* params) {
    int id = params->id;
    int* array = params->array;
    int size = params->size;

    auto& arrayMutex = params->arrayMutex;
    auto& coutMutex = params->coutMutex;
    auto& marked = params->markedIndices;
    auto& startCV = params->startCV;
    auto& pauseCV = params->pauseCV;

    auto& startSignal = params->startSignal;
    auto stopSignal = params->stopSignal;
    auto pausedSignal = params->pausedSignal;
    auto finishedSignal = params->finishedSignal;

    std::srand(id);

    {
        std::unique_lock<std::mutex> lock(arrayMutex);
        startCV.wait(lock, [&] { return startSignal.load(); });
    }

    while (!stopSignal->load()) {
        int markedCount = 0;
        bool blocked = false;

        while (!blocked && !stopSignal->load()) {
            int r = std::rand();
            int index = r % size;

            {
                std::lock_guard<std::mutex> lock(arrayMutex);
                if (array[index] == 0) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(5));
                    array[index] = id;
                    marked.push_back(index);
                    std::this_thread::sleep_for(std::chrono::milliseconds(5));
                    ++markedCount;
                }
                else {
                    {
                        std::lock_guard<std::mutex> coutLock(coutMutex);
                        std::cout << "[Marker " << id << "] Blocked on index " << index
                            << ". Marked count: " << markedCount << "\n";
                    }
                    pausedSignal->store(true);
                    blocked = true;
                }
            }

            if (blocked) {
                std::unique_lock<std::mutex> lock(arrayMutex);
                pauseCV.wait(lock, [&] {
                    return stopSignal->load() || !pausedSignal->load();
                    });
            }
        }
    }

    {
        std::lock_guard<std::mutex> lock(arrayMutex);
        for (int idx : marked) {
            array[idx] = 0;
        }
        finishedSignal->store(true);
    }
}
