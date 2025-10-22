# Thread-Safe HashMap in C++

A high-performance, thread-safe hash map implementation in C++ featuring fine-grained read-write locks.

## Features

- **Thread-Safe**: Concurrent reads and exclusive writes using custom read-write locks
- **Fine-Grained Locking**: One lock per bucket for maximum concurrency
- **Writer-Preference**: Prevents reader starvation of writers
- **Function Caching**: Built-in support for caching expensive function calls with custom types

## Architecture

### Components

- **RWLock**: Custom read-write lock with writer preference
- **HashMap**: Template-based hash map with separate chaining
- **HashCombiner**: Custom hash combiner for key hashing

### Thread Safety Design

```cpp
HashMap<std::string, int, 1000> map;

// Multiple readers can access different buckets concurrently
auto value1 = map.lookup_k("key1");  // Thread 1
auto value2 = map.lookup_k("key2");  // Thread 2 (concurrent if different buckets)

// Writers get exclusive access to their bucket
map.insert_kv("key3", 42);  // Thread 3 (exclusive for this bucket)
```

## Usage

### Basic Operations

```cpp
#include "hashmap/hashmap.h"

HashMap<std::string, int, 100> map;

// Insert key-value pairs
map.insert_kv("apple", 5);
map.insert_kv("banana", 3);

// Lookup values
auto result = map.lookup_k("apple");
if (result) {
    std::cout << "Found: " << result.value() << std::endl;
}

// Delete keys
bool deleted = map.delete_k("banana");
```

### Template Parameters

```cpp
template <typename K, typename V, int SZ = 1000>
class HashMap
```

- `K`: Key type (must be hashable with `std::hash`)
- `V`: Value type
- `SZ`: Number of buckets (default: 1000)

### Advanced Usage: Function Caching

The HashMap can be used to cache expensive function calls with custom types:

```cpp
#include "hashmap/hashcombiner.h"

// Custom type for caching
class StockData {
public:
    StockData(const std::string& name, uint64_t ts_before, uint64_t ts_after, uint32_t price_change)
        : tick_name(name), timestamp_before(ts_before), timestamp_after(ts_after), change_in_price(price_change) {}
    
    bool operator==(const StockData& other) const {
        return tick_name == other.tick_name && timestamp_before == other.timestamp_before && 
               timestamp_after == other.timestamp_after && change_in_price == other.change_in_price;
    }
    
    double get_ratio() const {
        return (double)change_in_price / (timestamp_after - timestamp_before);
    }
    
private:
    std::string tick_name;
    uint64_t timestamp_before, timestamp_after;
    uint32_t change_in_price;
    friend struct std::hash<StockData>;
};

// Custom hash specialization
template<>
struct std::hash<StockData> {
    size_t operator()(const StockData& obj) const {
        HashCombiner combiner;
        combiner.combine(obj.tick_name);
        combiner.combine(obj.timestamp_before);
        combiner.combine(obj.timestamp_after);
        combiner.combine(obj.change_in_price);
        return combiner;
    }
};

// Function cache implementation
class StockAnalyzer {
public:
    double calculate_ratio(const StockData& data) const {
        // Check cache first
        auto cached = cache.lookup_k(data);
        if (cached) {
            return cached.value();  // Cache hit!
        }
        
        // Expensive computation
        double ratio = data.get_ratio();
        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Simulate work
        
        // Store in cache
        cache.insert_kv(data, ratio);
        return ratio;
    }
    
private:
    mutable HashMap<StockData, double, 1000> cache;
};
```

## Building and Testing

### Prerequisites

- C++17 compatible compiler
- CMake 3.16+
- pthread support

### Build

```bash
mkdir build
cd build
cmake ..
make
```

### Run Tests

```bash
# Run all tests
ctest --verbose

# Run specific test categories
ctest -L locks     # Lock tests only
ctest -L hashmap   # HashMap tests only
```

### Run Demo

```bash
# Basic HashMap demo
./main

# Function caching demo
./cache
```

## Performance

The implementation achieves high concurrency through:

- **Bucket-level locking**: Different buckets can be accessed concurrently
- **Read-write locks**: Multiple concurrent readers per bucket
- **Writer preference**: Prevents reader starvation of writers
- **Lock-free hash computation**: Hash calculation outside critical sections

## Project Structure

```
hash-map-multithread/
├── src/
│   ├── locks/
│   │   ├── rw_lock.h          # Read-write lock interface
│   │   └── rw_lock.cpp        # Read-write lock implementation
│   └── hashmap/
│       └── hashmap.h          # Template hash map implementation
├── tests/
│   ├── locks/
│   │   └── rw_lock_test.cpp   # Lock functionality tests
│   └── hashmap/
│       └── hashmap_test.cpp   # Concurrent hashmap tests
├── app/
│   ├── main.cpp               # Basic HashMap demo
│   └── cache.cpp              # Function caching demo
└── CMakeLists.txt             # Build configuration
```
