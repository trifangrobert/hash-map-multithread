#pragma once
#include <string>

class HashCombiner {
public:
    size_t hash_value = 0;
    
    template<typename T>
    void combine(const T& value) {
        std::hash<T> hasher;
        hash_value ^= hasher(value) + 0x9e3779b9 + (hash_value << 6) + (hash_value >> 2);
    }
    
    operator size_t() const {
        return hash_value;
    }
};