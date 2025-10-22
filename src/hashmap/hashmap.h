#pragma once
#include <vector>
#include <functional>
#include <optional>
#include <algorithm>

#include "locks/rw_lock.h"

template <typename K, typename V, int SZ = 1000, typename Hash = std::hash<K>>
class HashMap {
public:
    size_t hash_fn(const K& key) {
        return hasher(key) % SZ;
    }

    HashMap() {
        hash_map.resize(SZ);
        rwlocks.resize(SZ);
    }
    void insert_kv(const K& key, const V& value) {
        size_t hash_val = hash_fn(key);
        RWLock& curr_lock = rwlocks[hash_val];
        
        curr_lock.write([this, &key, &value]() {
            size_t hash_val = hash_fn(key);
            auto& bucket = hash_map[hash_val];
            bool found = false;
            for (auto& [k, v] : bucket) {
                if (k == key) {
                    v = value;
                    found = true;
                    break;
                }
            }
            if (!found) {
                hash_map[hash_val].emplace_back(key, value);
            }
        });
    }

    std::optional<V> lookup_k(const K& key) {
        size_t hash_val = hash_fn(key);
        RWLock& curr_lock = rwlocks[hash_val];
        return curr_lock.read([this, &key]() -> std::optional<V> {
            size_t hash_val = hash_fn(key);
            for (const auto& [k, v] : hash_map[hash_val]) {
                if (k == key) {
                    return v;
                }
            }
            return std::nullopt;
        });
    }

    bool delete_k(const K& key) {
        size_t hash_val = hash_fn(key);
        RWLock& curr_lock = rwlocks[hash_val];
        return curr_lock.write([this, &key]() -> bool {
            size_t hash_val = hash_fn(key);
            auto& bucket = hash_map[hash_val];
            
            auto it = std::remove_if(bucket.begin(), bucket.end(), [&key](const std::pair<K, V>& curr) {
                return curr.first == key;
            });

            bool found = (it != bucket.end());
            bucket.erase(it, bucket.end());
            
            return found;
        });
    }

private:
    Hash hasher;
    std::vector<std::vector<std::pair<K, V>>> hash_map;
    std::vector<RWLock> rwlocks;
};