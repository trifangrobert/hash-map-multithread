#include <thread>
#include <vector>
#include <functional>
#include <sstream>
#include <iostream>
#include <chrono>
#include <random>

#include "locks/rw_lock.h"

int main() {
    std::cout << "=== RW Lock Basic Test ===" << std::endl;
    
    RWLock rwlock;

    std::atomic<int> reader_count{0};
    std::atomic<int> writer_count{0};

    std::function<void(int)> read_fn = [&reader_count](int thread_id) {
        reader_count++;
        std::ostringstream os;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        os << "Thread  " << thread_id << " reading\n";
        std::cout << os.str();
    };

    std::function<void(int)> write_fn = [&writer_count](int thread_id) {
        writer_count++;
        std::ostringstream os;
        os << "Thread " << thread_id << " writing\n";
        std::cout << os.str();
    };

    auto start_time = std::chrono::steady_clock::now();

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 1);

    std::vector<std::thread> threads;

    // size_t readers = 10;
    // size_t writers = 10;
    // for (int i = 0; i < readers; ++i) {
    //     threads.emplace_back([&rwlock, read_fn, i]() {
    //         rwlock.read(read_fn, i);
    //     });
    // }

    // for (int i = 0; i < writers; ++i) {
    //     threads.emplace_back([&rwlock, write_fn, i]() {
    //         rwlock.write(write_fn, i);
    //     });
    // }

    size_t N = 10;
    
    for (int i = 0; i < N; ++i) {
        if (dis(gen)) {
            threads.emplace_back([&rwlock, read_fn, i]() {
                rwlock.read(read_fn, i);
            });
        }
        else {
            threads.emplace_back([&rwlock, write_fn, i]() {
                rwlock.write(write_fn, i);
            });
        }
    }

    for (auto& t : threads) {
        t.join();
    }
    
    auto end_time = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    std::cout << "=== Test Results ===" << std::endl;
    std::cout << "Total threads created: " << N << std::endl;
    std::cout << "Readers completed: " << reader_count.load() << std::endl;
    std::cout << "Writers completed: " << writer_count.load() << std::endl;
    std::cout << "Total completed: " << (reader_count.load() + writer_count.load()) << std::endl;
    std::cout << "Duration: " << duration.count() << "ms" << std::endl;
    
    bool success = (reader_count.load() + writer_count.load()) == N;
    
    if (success) {
        std::cout << "✅ TEST PASSED" << std::endl;
        return 0;
    } else {
        std::cout << "❌ TEST FAILED - Not all threads completed" << std::endl;
        return 1;
    }
}