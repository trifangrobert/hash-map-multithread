#include <string>
#include <iostream>

#include "hashmap/hashmap.h"

int main() {
    HashMap<std::string, int, 10> h;
    h.insert_kv("apple", 4);
    h.insert_kv("banana", 10);
    h.insert_kv("pear", 7);
    h.insert_kv("pineapple", 3);
    h.insert_kv("kiwi", 9);

    auto print_lookup = [&](const std::string& key) {
        std::optional<int> ret;
        ret = h.lookup_k(key);
        if (ret) {
            std::cout << ret.value() << "\n";
        }
        else {
            std::cout << "Not found\n";
        }
    };

    print_lookup("apple");
    print_lookup("coconut");

    h.delete_k("apple");
    print_lookup("apple");
    return 0;
}