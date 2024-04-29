#ifndef MARKETORDER_HPP
#define MARKETORDER_HPP

#include "order.hpp"

class MarketOrder: public Order {
public:

    MarketOrder(): Order() {};

    MarketOrder(int order_id) 
    : Order(order_id, Order::Type::MARKET, Order::TimeInForce::IOC) {};

private:

    friend std::ostream& operator<<(std::ostream& os, const MarketOrder& order)
    {
        os << order.timestamp_created << " [MarketOrder] Id: " << order.id << " " << order.ticker << " " << order.remaining_quantity;
        return os;
    }

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & boost::serialization::base_object<Order>(*this);
    }
};

typedef std::shared_ptr<MarketOrder> MarketOrderPtr;

#endif