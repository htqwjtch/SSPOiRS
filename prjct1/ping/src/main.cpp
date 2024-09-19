#include <thread>
#include <vector>

#include "threadService/threadservice.h"

int main(int argc, char** argv) {
    const int threadAmount = argc - 1;
    std::vector<std::thread> threads;
    for (int i = 0; i < threadAmount; i++) {
        threads.emplace_back(run_thread, argc, argv, i + 1);
    }

    for (auto& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    return 0;
}