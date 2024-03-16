#ifndef ORDER_FACTORY_HPP
#define ORDER_FACTORY_HPP

#include "order.hpp"
#include "../message/limit_order_message.hpp"

class OrderFactory
{
public:

    OrderFactory() = default;

    OrderPtr createLimitOrder(LimitOrderMessagePtr msg)
    {
        std::shared_ptr<Order> order = std::make_shared<Order>(++order_id_);
        order->sender_id = msg->sender_id;
        order->ticker = msg->ticker;
        order->side = msg->side;
        order->price = msg->price;
        order->remaining_quantity = msg->quantity;
        return order;
    }

    int getNumberOfOrders() const
    {
        return order_id_;
    }

private:

    int order_id_ = 0;
};

#endif