#ifndef MARKETORDER_HPP
#define MARKETORDER_HPP

#include "order.hpp"

class MarketOrder: public Order {
public:

    MarketOrder(int order_id) 
    : Order(order_id, Order::Type::MARKET) {};
    
    MarketOrder() = delete;

private:

    friend std::ostream& operator<<(std::ostream& os, const MarketOrder& order)
    {
        os << order.timestamp << " [MarketOrder] Id: " << order.id << " " << order.ticker << " " << order.remaining_quantity;
        return os;
    }
};

typedef std::shared_ptr<MarketOrder> MarketOrderPtr;

#endif