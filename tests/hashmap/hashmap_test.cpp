#include <thread>
#include <vector>
#include <random>
#include <chrono>
#include <iostream>
#include <atomic>
#include <unordered_set>

#include "hashmap/hashmap.h"

class HashMapTester {
public:
    HashMap<std::string, int, 100> hashmap;
    std::atomic<int> insert_count{0};
    std::atomic<int> lookup_count{0};  
    std::atomic<int> delete_count{0};
    std::atomic<int> successful_lookups{0};
    std::atomic<int> successful_deletes{0};

    void test_concurrent_operations() {
        std::cout << "=== Concurrent HashMap Test ===" << std::endl;
        
        const int num_threads = 8;
        const int operations_per_thread = 100;
        
        std::vector<std::thread> threads;
        
        for (int thread_id = 0; thread_id < num_threads; ++thread_id) {
            threads.emplace_back([this, thread_id, operations_per_thread]() {
                worker_thread(thread_id, operations_per_thread);
            });
        }
        
        auto start_time = std::chrono::steady_clock::now();
        
        for (auto& t : threads) {
            t.join();
        }
        
        auto end_time = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        
        print_results(duration.count(), num_threads, operations_per_thread);
    }
    
    void test_reader_writer_scenario() {
        std::cout << "\n=== Reader-Heavy Scenario ===" << std::endl;
        
        // Pre-populate with some data
        for (int i = 0; i < 50; ++i) {
            hashmap.insert_kv("key_" + std::to_string(i), i * 10);
        }
        
        const int num_readers = 6;
        const int num_writers = 2;
        const int operations = 200;
        
        std::vector<std::thread> threads;
        
        // Reader threads
        for (int i = 0; i < num_readers; ++i) {
            threads.emplace_back([this, i, operations]() {
                reader_thread(i, operations);
            });
        }
        
        // Writer threads  
        for (int i = 0; i < num_writers; ++i) {
            threads.emplace_back([this, i, operations]() {
                writer_thread(i, operations);
            });
        }
        
        auto start_time = std::chrono::steady_clock::now();
        
        for (auto& t : threads) {
            t.join();
        }
        
        auto end_time = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        
        std::cout << "Reader-heavy test completed in " << duration.count() << "ms" << std::endl;
        std::cout << "Successful lookups: " << successful_lookups.load() << std::endl;
    }

private:
    void worker_thread(int thread_id, int operations) {
        std::random_device rd;
        std::mt19937 gen(rd() + thread_id);  // seed with thread_id for diversity
        std::uniform_int_distribution<> op_dis(0, 2);  // 0=insert, 1=lookup, 2=delete
        std::uniform_int_distribution<> key_dis(0, 199);  // Key range
        
        for (int i = 0; i < operations; ++i) {
            int operation = op_dis(gen);
            int key_num = key_dis(gen);
            std::string key = "key_" + std::to_string(key_num);
            
            switch (operation) {
                case 0: {  // Insert
                    int value = thread_id * 1000 + i;
                    hashmap.insert_kv(key, value);
                    insert_count++;
                    break;
                }
                case 1: {  // Lookup
                    auto result = hashmap.lookup_k(key);
                    lookup_count++;
                    if (result.has_value()) {
                        successful_lookups++;
                    }
                    break;
                }
                case 2: {  // Delete
                    bool deleted = hashmap.delete_k(key);
                    delete_count++;
                    if (deleted) {
                        successful_deletes++;
                    }
                    break;
                }
            }
            
            std::this_thread::sleep_for(std::chrono::microseconds(10));
        }
    }
    
    void reader_thread(int thread_id, int operations) {
        std::random_device rd;
        std::mt19937 gen(rd() + thread_id + 1000);
        std::uniform_int_distribution<> key_dis(0, 49);  // Keys that exist
        
        for (int i = 0; i < operations; ++i) {
            std::string key = "key_" + std::to_string(key_dis(gen));
            auto result = hashmap.lookup_k(key);
            lookup_count++;
            if (result.has_value()) {
                successful_lookups++;
            }
        }
    }
    
    void writer_thread(int thread_id, int operations) {
        for (int i = 0; i < operations; ++i) {
            std::string key = "writer_" + std::to_string(thread_id) + "_" + std::to_string(i);
            hashmap.insert_kv(key, thread_id * 1000 + i);
            insert_count++;
            
            // Occasionally update existing keys
            if (i % 10 == 0) {
                std::string existing_key = "key_" + std::to_string(i % 50);
                hashmap.insert_kv(existing_key, 9999);
            }
        }
    }
    
    void print_results(long duration_ms, int num_threads, int ops_per_thread) {
        int total_ops = insert_count.load() + lookup_count.load() + delete_count.load();
        
        std::cout << "\n=== Test Results ===" << std::endl;
        std::cout << "Threads: " << num_threads << std::endl;
        std::cout << "Duration: " << duration_ms << "ms" << std::endl;
        std::cout << "Total operations: " << total_ops << std::endl;
        std::cout << "Operations per second: " << (total_ops * 1000.0 / duration_ms) << std::endl;
        std::cout << std::endl;
        std::cout << "Insert operations: " << insert_count.load() << std::endl;
        std::cout << "Lookup operations: " << lookup_count.load() << std::endl;
        std::cout << "Delete operations: " << delete_count.load() << std::endl;
        std::cout << std::endl;
        std::cout << "Successful lookups: " << successful_lookups.load() 
                  << "/" << lookup_count.load() << " ("
                  << (100.0 * successful_lookups.load() / std::max(1, lookup_count.load())) << "%)" << std::endl;
        std::cout << "Successful deletes: " << successful_deletes.load() 
                  << "/" << delete_count.load() << " ("
                  << (100.0 * successful_deletes.load() / std::max(1, delete_count.load())) << "%)" << std::endl;
    }
};

int main() {
    try {
        HashMapTester tester;
        
        tester.test_concurrent_operations();
        
        tester.insert_count = 0;
        tester.lookup_count = 0;
        tester.delete_count = 0;
        tester.successful_lookups = 0;
        tester.successful_deletes = 0;
        
        tester.test_reader_writer_scenario();
        
        std::cout << "\n✅ All tests completed successfully!" << std::endl;
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Test failed: " << e.what() << std::endl;
        return 1;
    }
}