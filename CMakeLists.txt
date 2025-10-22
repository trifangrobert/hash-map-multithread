cmake_minimum_required(VERSION 3.16)
project(ThreadSafeHashMap)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)

# Enable testing
enable_testing()

add_library(rw_lock
    src/locks/rw_lock.cpp
    src/locks/rw_lock.h
)

# Header-only library for hashmap
add_library(hashmap INTERFACE)
target_include_directories(hashmap INTERFACE src)
target_link_libraries(hashmap INTERFACE rw_lock)

target_link_libraries(rw_lock Threads::Threads)
target_include_directories(rw_lock PUBLIC src)

add_executable(main app/main.cpp)
target_link_libraries(main rw_lock)
target_include_directories(main PRIVATE src)

add_executable(cache app/cache.cpp)
target_link_libraries(cache hashmap Threads::Threads)
target_include_directories(cache PRIVATE src)

# Test executables
add_executable(test_rw_lock_basic tests/locks/rw_lock_test.cpp)
target_link_libraries(test_rw_lock_basic rw_lock)
target_include_directories(test_rw_lock_basic PRIVATE src)

add_executable(test_hashmap tests/hashmap/hashmap_test.cpp)
target_link_libraries(test_hashmap hashmap)
target_include_directories(test_hashmap PRIVATE src)

# Register tests with CTest
add_test(NAME RWLockBasicTest COMMAND test_rw_lock_basic)
add_test(NAME HashMapTest COMMAND test_hashmap)

# Set test properties (optional)
set_tests_properties(RWLockBasicTest PROPERTIES
    TIMEOUT 30  # Test timeout in seconds
    LABELS "locks;unit"  # Test categories
)

set_tests_properties(HashMapTest PROPERTIES
    TIMEOUT 60  # Longer timeout for concurrent tests
    LABELS "hashmap;integration"
)

# Custom targets for convenience
add_custom_target(run_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --verbose
    DEPENDS test_rw_lock_basic
    COMMENT "Running all tests with verbose output"
)

add_custom_target(run_lock_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --verbose -L locks
    DEPENDS test_rw_lock_basic
    COMMENT "Running only lock tests"
)