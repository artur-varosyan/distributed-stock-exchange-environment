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

    SyncMessageQueue()
    : queue_{},
      lock_{},
      cv{}
    {
    };

    /** Pushes the value to the end of the queue. */
    void push(T value);

    /** Wait until present and pops the value from the start of the queue. */
    T pop();

    /** Returns the size of the queue. */
    unsigned int size();

private:

    std::queue<T> queue_;
    std::mutex lock_;
    std::condition_variable cv_;
};

#endif