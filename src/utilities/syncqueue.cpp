#include "syncqueue.hpp"

template <typename T>
void SyncQueue<T>::push(T value)
{
    std::unique_lock<std::mutex> lock(lock_); 
    queue_.push(value);
    cv_.notify_all();
};

template <typename T>
T SyncQueue<T>::pop()
{
    // Wait until new message added to the queue
    std::unique_lock<std::mutex> lock(lock_); 
    cv_.wait(lock, [this]{ return !queue_.empty(); });

    // Return the value at the front of the queue
    return queue_.pop();
};

template <typename T>
unsigned int SyncQueue<T>::size()
{
    return queue_.size();
};