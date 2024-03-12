#ifndef ORDER_QUEUE_HPP
#define ORDER_QUEUE_HPP

#include <queue>
#include <optional>

#include "order.hpp"

/** A FIFO queue with price-time priority for currently active orders. */
class OrderQueue: public std::priority_queue<OrderPtr, std::vector<OrderPtr>, std::function<bool(OrderPtr, OrderPtr)>> {
public:

    OrderQueue(Order::Side side)
    : std::priority_queue<OrderPtr, std::vector<OrderPtr>, std::function<bool(OrderPtr, OrderPtr)>>()
    {
        if (side == Order::Side::BID) 
        {
            // Sort bids in descending order
            this->comp = [](OrderPtr a, OrderPtr b) { 
                if (a->price != b->price) {
                    return a->price >= b->price; 
                }
                else {
                    // Compare timestamps if prices are equal
                    return a->timestamp <= b->timestamp;
                }
            };
        } else {
            // Sort asks in ascending order
            this->comp = [](OrderPtr a, OrderPtr b) { 
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
    std::optional<OrderPtr> find(int order_id);

    /** Removes and returns the order with the given id if present in the queue. */
    std::optional<OrderPtr> remove(int order_id);

};

#endif