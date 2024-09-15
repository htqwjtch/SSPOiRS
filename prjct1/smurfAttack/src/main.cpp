#include <thread>
#include <vector>

#include "threadService/threadservice.h"

int main(int argc, char** argv) {
    std::string threadAmountString = std::string(argv[2]);
    const int threadAmount = std::stoi(threadAmountString);
    std::vector<std::thread> threads;
    for (int i = 0; i < threadAmount; i++) {
        threads.emplace_back(run_thread, argc, argv);
    }

    for (auto& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    return 0;
}
