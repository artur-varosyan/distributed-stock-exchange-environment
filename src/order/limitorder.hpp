#ifndef LIMITORDER_HPP
#define LIMITORDER_HPP

#include "order.hpp"

class LimitOrder: public Order {
public:

    LimitOrder(int order_id)
    : Order(order_id, Order::Type::LIMIT) {};

    LimitOrder() = delete;

    int price;

private:

        friend std::ostream& operator<<(std::ostream& os, const LimitOrder& order)
        {
            os << order.timestamp << " [LimitOrder] Id: " << order.id << " " << order.ticker << " " << order.remaining_quantity << " @ $" << order.price;
            return os;
        }
};

typedef std::shared_ptr<LimitOrder> LimitOrderPtr;

#endif