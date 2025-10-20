# Thread-Safe HashMap in C++

A high-performance, thread-safe hash map implementation in C++ featuring fine-grained read-write locks.

## Features

- **Thread-Safe**: Concurrent reads and exclusive writes using custom read-write locks
- **Fine-Grained Locking**: One lock per bucket for maximum concurrency
- **Modern C++**: Uses `std::optional`, templates, and perfect forwarding
- **Writer-Preference**: Prevents reader starvation of writers
- **Exception-Safe**: RAII-based lock management with proper exception handling

## Architecture

### Components

- **RWLock**: Custom read-write lock with writer preference
- **HashMap**: Template-based hash map with separate chaining
- **Comprehensive Tests**: Multi-threaded test suite with CTest integration

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
./main
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
│   └── main.cpp               # Demo application
└── CMakeLists.txt             # Build configuration
```

## Contributing

Feel free to open issues or submit pull requests for improvements!

## License

This project is for educational purposes.