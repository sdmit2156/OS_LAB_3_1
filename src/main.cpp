#include "marker.h"
#include <iostream>
#include <vector>
#include <thread>
#include <atomic>
#include <memory>

int main() {
    int size, numMarkers;

    std::cout << "Enter array size: ";
    std::cin >> size;
    int* array = new int[size] {};

    std::cout << "Enter number of marker threads: ";
    std::cin >> numMarkers;

    std::vector<std::thread> threads(numMarkers);
    std::vector<std::vector<int>> marked(numMarkers);
    std::vector<MarkerParams> params;
    params.reserve(numMarkers);

    std::mutex arrayMutex;
    std::mutex coutMutex;

    std::condition_variable startCV;
    std::condition_variable pauseCV;
    std::atomic<bool> startSignal = false;

    std::vector<std::shared_ptr<std::atomic<bool>>> stopSignals(numMarkers);
    std::vector<std::shared_ptr<std::atomic<bool>>> pausedSignals(numMarkers);
    std::vector<std::shared_ptr<std::atomic<bool>>> finishedSignals(numMarkers);

    for (int i = 0; i < numMarkers; ++i) {
        stopSignals[i] = std::make_shared<std::atomic<bool>>(false);
        pausedSignals[i] = std::make_shared<std::atomic<bool>>(false);
        finishedSignals[i] = std::make_shared<std::atomic<bool>>(false);

        params.emplace_back(MarkerParams{
            i + 1,
            array,
            size,
            arrayMutex,
            coutMutex,
            marked[i],
            startCV,
            pauseCV,
            startSignal,
            stopSignals[i],
            pausedSignals[i],
            finishedSignals[i]
            });

        threads[i] = std::thread(MarkerThread, &params[i]);
    }

    {
        std::lock_guard<std::mutex> lock(arrayMutex);
        startSignal = true;
    }
    startCV.notify_all();

    int active = numMarkers;
    while (active > 0) {
        bool allPaused = false;
        while (!allPaused) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            allPaused = true;
            for (int i = 0; i < numMarkers; ++i) {
                if (!finishedSignals[i]->load() && !pausedSignals[i]->load()) {
                    allPaused = false;
                    break;
                }
            }
        }

        {
            std::lock_guard<std::mutex> lock(coutMutex);
            std::cout << "Current array state:\n";
            for (int i = 0; i < size; ++i)
                std::cout << array[i] << " ";
            std::cout << "\n";
        }

        int toStop = -1;
        while (true) {
            std::cout << "Enter marker thread to terminate (1-" << numMarkers << "): ";
            std::cin >> toStop;
            toStop -= 1;
            if (toStop < 0 || toStop >= numMarkers || finishedSignals[toStop]->load()) {
                std::cout << "Invalid thread number.\n";
            }
            else {
                break;
            }
        }

        stopSignals[toStop]->store(true);
        pausedSignals[toStop]->store(false);
        pauseCV.notify_all();

        threads[toStop].join();
        --active;

        {
            std::lock_guard<std::mutex> lock(coutMutex);
            std::cout << "Array after terminating marker " << (toStop + 1) << ":\n";
            for (int i = 0; i < size; ++i)
                std::cout << array[i] << " ";
            std::cout << "\n";
        }

        for (int i = 0; i < numMarkers; ++i) {
            if (!finishedSignals[i]->load()) {
                pausedSignals[i]->store(false);
            }
        }
        pauseCV.notify_all();
    }

    delete[] array;
    std::cout << "All marker threads terminated. Exiting.\n";
    return 0;
}
