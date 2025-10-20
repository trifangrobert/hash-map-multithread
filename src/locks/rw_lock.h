#pragma once
#include <cstddef>
#include <pthread.h>
#include <functional>
#include <type_traits>


class RWLock {
public:
    RWLock();
    ~RWLock();

    template <typename Callable, typename ...Args>
    auto read(Callable&& func, Args&&... args) -> decltype(func(args...)) {
        reader_lock();
        try {
            if constexpr (std::is_void_v<decltype(func(args...))>) {
                func(std::forward<Args>(args)...);
                reader_unlock();
            } else {
                auto result = func(std::forward<Args>(args)...);
                reader_unlock();
                return result;
            }
        }
        catch (...) {
            reader_unlock();
            throw;
        }
    }

    template <typename Callable, typename ...Args>
    auto write(Callable&& func, Args&&... args) -> decltype(func(args...)) {
        writer_lock();
        try {
            if constexpr (std::is_void_v<decltype(func(args...))>) {
                func(std::forward<Args>(args)...);
                write_unlock();
            } else {
                auto result = func(std::forward<Args>(args)...);
                write_unlock();
                return result;
            }
        }
        catch (...) {
            write_unlock();
            throw;
        }
    }

private:
    void reader_lock();
    void reader_unlock();

    void writer_lock();
    void write_unlock();
private:
    size_t readers_, writers_;
    size_t waiting_readers_, waiting_writers_;
    pthread_mutex_t lock;
    pthread_cond_t read_cond, write_cond;
};