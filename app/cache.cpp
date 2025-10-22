#include <iostream>
#include <string>
#include <functional>
#include <chrono>
#include <vector>
#include <thread>

#include "hashmap/hashcombiner.h"
#include "hashmap/hashmap.h"

class custom_t {
public:
    custom_t(const std::string& name, uint64_t ts_before, uint64_t ts_after, uint32_t p) 
        : tick_name(name), timestamp_before(ts_before), timestamp_after(ts_after), change_in_price(p) {}
    
    bool operator==(const custom_t& other) const {
        return tick_name == other.tick_name && 
               timestamp_before == other.timestamp_before && 
               timestamp_after == other.timestamp_after && 
               change_in_price == other.change_in_price;
    }
    
    friend struct std::hash<custom_t>;

    double get_ratio() const {
        return (double)change_in_price / (timestamp_after - timestamp_before);
    }
    
private:
    std::string tick_name;
    uint64_t timestamp_before;
    uint64_t timestamp_after;
    uint32_t change_in_price;
};


template<>
struct std::hash<custom_t> {
    size_t operator()(const custom_t& obj) const {
        HashCombiner combiner;
        combiner.combine(obj.tick_name);
        combiner.combine(obj.timestamp_before);
        combiner.combine(obj.timestamp_after);
        combiner.combine(obj.change_in_price);
        return combiner;
    }
};

class CacheFunction {
public:
    double change_ratio(const custom_t& snapshot) const {
        std::optional<double> ret = hash_map.lookup_k(snapshot);
        if (ret) {
            return ret.value();
        }

        double ratio = snapshot.get_ratio();
        hash_map.insert_kv(snapshot, ratio);
        // std::this_thread::sleep_for(std::chrono::milliseconds(100));
        return ratio;
    }
private:
    mutable HashMap<custom_t, double, 1000, std::hash<custom_t>> hash_map;
};


int main() {
    std::cout << "=== CacheFunction Demo ===" << std::endl;
    
    CacheFunction cache;
    
    // let copilot create some test data
    custom_t snapshot1("AAPL", 1000, 2000, 150);  // Apple stock: +150 cents over 1000ms
    custom_t snapshot2("MSFT", 1500, 2500, 200);  // Microsoft: +200 cents over 1000ms  
    custom_t snapshot3("AAPL", 1000, 2000, 150);  // Same as snapshot1 (should hit cache)
    custom_t snapshot4("GOOGL", 800, 1800, 500);  // Google: +500 cents over 1000ms
    
    std::cout << "\n--- First calls (should compute and cache) ---" << std::endl;
    
    auto start_time = std::chrono::high_resolution_clock::now();
    double ratio1 = cache.change_ratio(snapshot1);
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration1 = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    std::cout << "AAPL ratio: " << ratio1 << " (computed in " << duration1.count() << " μs)" << std::endl;
    
    start_time = std::chrono::high_resolution_clock::now();
    double ratio2 = cache.change_ratio(snapshot2);
    end_time = std::chrono::high_resolution_clock::now();
    auto duration2 = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    std::cout << "MSFT ratio: " << ratio2 << " (computed in " << duration2.count() << " μs)" << std::endl;
    
    start_time = std::chrono::high_resolution_clock::now();
    double ratio4 = cache.change_ratio(snapshot4);
    end_time = std::chrono::high_resolution_clock::now();
    auto duration4 = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    std::cout << "GOOGL ratio: " << ratio4 << " (computed in " << duration4.count() << " μs)" << std::endl;
    
    std::cout << "\n--- Cache hit test (should be faster) ---" << std::endl;
    
    start_time = std::chrono::high_resolution_clock::now();
    double ratio3 = cache.change_ratio(snapshot3);  // Same as snapshot1 - should hit cache!
    end_time = std::chrono::high_resolution_clock::now();
    auto duration3 = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    std::cout << "AAPL ratio (cached): " << ratio3 << " (retrieved in " << duration3.count() << " μs)" << std::endl;
    
    std::cout << "\n--- Results verification ---" << std::endl;
    std::cout << "First AAPL call: " << ratio1 << std::endl;
    std::cout << "Cached AAPL call: " << ratio3 << std::endl;
    std::cout << "Results match: " << (ratio1 == ratio3 ? "✅ YES" : "❌ NO") << std::endl;
    
    std::cout << "\n--- Performance comparison ---" << std::endl;
    std::cout << "First computation: " << duration1.count() << " μs" << std::endl;
    std::cout << "Cache retrieval: " << duration3.count() << " μs" << std::endl;
    
    if (duration3.count() < duration1.count()) {
        double speedup = (double)duration1.count() / duration3.count();
        std::cout << "Cache is " << speedup << "x faster! 🚀" << std::endl;
    }
    
    std::vector<custom_t> stocks;
    for (int i = 0; i < 10; ++i) {
        stocks.emplace_back("STOCK" + std::to_string(i), 1000, 2000 + i*100, 100 + i*50);
    }
    
    auto cache_start = std::chrono::high_resolution_clock::now();
    for (const auto& stock : stocks) {
        cache.change_ratio(stock);
    }
    auto cache_mid = std::chrono::high_resolution_clock::now();
    
    for (const auto& stock : stocks) {
        cache.change_ratio(stock);
    }
    auto cache_end = std::chrono::high_resolution_clock::now();
    
    auto first_pass_time = std::chrono::duration_cast<std::chrono::microseconds>(cache_mid - cache_start);
    auto second_pass_time = std::chrono::duration_cast<std::chrono::microseconds>(cache_end - cache_mid);
    
    std::cout << "10 stocks first pass (compute): " << first_pass_time.count() << " μs" << std::endl;
    std::cout << "10 stocks second pass (cache): " << second_pass_time.count() << " μs" << std::endl;
    
    if (second_pass_time.count() > 0) {
        double batch_speedup = (double)first_pass_time.count() / second_pass_time.count();
        std::cout << "Batch cache speedup: " << batch_speedup << "x" << std::endl;
    }
    
    std::cout << "\n✅ Cache function demo completed!" << std::endl;
    return 0;
}