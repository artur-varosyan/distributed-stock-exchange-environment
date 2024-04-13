#ifndef LIMITORDER_HPP
#define LIMITORDER_HPP

#include "order.hpp"

class LimitOrder: public Order {
public:

    LimitOrder(): Order() {};

    LimitOrder(int order_id)
    : Order(order_id, Order::Type::LIMIT) {};

    int price;

private:

    friend std::ostream& operator<<(std::ostream& os, const LimitOrder& order)
    {
        os << order.timestamp << " [LimitOrder] Id: " << order.id << " " << order.ticker << " " << order.remaining_quantity << " @ $" << order.price;
        return os;
    }

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & boost::serialization::base_object<Order>(*this);
        ar & price;
    }
};

typedef std::shared_ptr<LimitOrder> LimitOrderPtr;

#endif