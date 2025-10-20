#include "rw_lock.h"

RWLock::RWLock() : readers_(0), waiting_readers_(0), writers_(0), waiting_writers_(0) {
    pthread_mutex_init(&lock, NULL);
    pthread_cond_init(&read_cond, NULL);
    pthread_cond_init(&write_cond, NULL);
}

RWLock::~RWLock() {
    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&read_cond);
    pthread_cond_destroy(&write_cond);
}

void RWLock::reader_lock() {
    pthread_mutex_lock(&lock);
    if (writers_ > 0 || waiting_writers_ > 0) {
        waiting_readers_ += 1;
        while (writers_ > 0 || waiting_writers_ > 0) {
            pthread_cond_wait(&read_cond, &lock);
        }
        waiting_readers_ -= 1;
    }
    readers_ += 1;
    pthread_mutex_unlock(&lock);
}

void RWLock::reader_unlock() {
    pthread_mutex_lock(&lock);
    readers_ -= 1;
    if (waiting_writers_ > 0) {
        pthread_cond_broadcast(&write_cond);
    }
    pthread_mutex_unlock(&lock);
}

void RWLock::writer_lock() {
    pthread_mutex_lock(&lock);
    if (readers_ > 0 || writers_ > 0) {
        waiting_writers_ += 1;
        while (readers_ > 0 || writers_ > 0) {
            pthread_cond_wait(&write_cond, &lock);
        }
        waiting_writers_ -= 1;
    }
    writers_ += 1;
    pthread_mutex_unlock(&lock);
}

void RWLock::write_unlock() {
    pthread_mutex_lock(&lock);
    writers_ -= 1;
    if (waiting_writers_ > 0) {
        pthread_cond_signal(&write_cond);
    }
    else if (waiting_readers_ > 0) {
        pthread_cond_broadcast(&read_cond);
    }
    pthread_mutex_unlock(&lock);
}