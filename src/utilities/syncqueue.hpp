#ifndef SYNC_QUEUE_HPP
#define SYNC_QUEUE_HPP

#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>

/** Thread-safe FIFO synchronised queue. */
template <typename T>
class SyncQueue : std::enable_shared_from_this<SyncQueue<T>>
{
public:

    SyncQueue()
    : queue_{},
      lock_{},
      cv_{}
    {
    };

    /** Pushes the value to the end of the queue. */
    void push(T value)
    {
      std::unique_lock<std::mutex> lock(lock_); 
      queue_.push(value);
      cv_.notify_all();
    };

    /** Wait until present and pops the value from the start of the queue. */
    T pop()
    {
      // Wait until new message added to the queue
      std::unique_lock<std::mutex> lock(lock_); 
      cv_.wait(lock, [this]{ return !queue_.empty(); });

      // Return the value at the front of the queue
      T value = queue_.front();
      queue_.pop();
      return value;
    };

    /** Returns the size of the queue. */
    unsigned int size()
    {
      return queue_.size();
    };

private:

    std::queue<T> queue_;
    std::mutex lock_;
    std::condition_variable cv_;
};

#endif