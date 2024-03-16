#ifndef ORDER_QUEUE_HPP
#define ORDER_QUEUE_HPP

#include <queue>
#include <optional>

#include "limitorder.hpp"

/** A FIFO queue with price-time priority for currently active orders. */
class OrderQueue: public std::priority_queue<LimitOrderPtr, std::vector<LimitOrderPtr>, std::function<bool(LimitOrderPtr, LimitOrderPtr)>> {
public:

    OrderQueue(Order::Side side)
    : std::priority_queue<LimitOrderPtr, std::vector<LimitOrderPtr>, std::function<bool(LimitOrderPtr, LimitOrderPtr)>>()
    {
        if (side == Order::Side::BID) 
        {
            // Sort bids in descending order
            this->comp = [](LimitOrderPtr a, LimitOrderPtr b) { 
                if (a->price != b->price) {
                    return a->price <= b->price; 
                }
                else {
                    // Compare timestamps if prices are equal
                    return a->timestamp <= b->timestamp;
                }
            };
        } else {
            // Sort asks in ascending order
            this->comp = [](LimitOrderPtr a, LimitOrderPtr b) { 
                if (a->price != b->price) {
                    return a->price >= b->price; 
                }
                else {
                    // Compare timestamps if prices are equal
                    return a->timestamp <= b->timestamp;
                }
             };
        }
    }

    /** Finds and returns the order with the given id if present in the queue. */
    std::optional<LimitOrderPtr> find(int order_id);

    /** Removes and returns the order with the given id if present in the queue. */
    std::optional<LimitOrderPtr> remove(int order_id);

};

#endif