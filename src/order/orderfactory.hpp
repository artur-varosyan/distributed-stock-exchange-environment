#ifndef ORDER_FACTORY_HPP
#define ORDER_FACTORY_HPP

#include "limitorder.hpp"
#include "marketorder.hpp"
#include "../message/limit_order_message.hpp"
#include "../message/market_order_message.hpp"

class OrderFactory
{
public:

    OrderFactory() = default;

    LimitOrderPtr createLimitOrder(LimitOrderMessagePtr msg)
    {
        LimitOrderPtr order = std::make_shared<LimitOrder>(++order_id_);
        order->sender_id = msg->sender_id;
        order->client_order_id = msg->client_order_id;
        order->ticker = msg->ticker;
        order->side = msg->side;
        order->time_in_force = msg->time_in_force;
        order->price = msg->price;
        order->status = Order::Status::NEW;
        order->remaining_quantity = msg->quantity;
        order->cumulative_quantity = 0;
        order->avg_price = 0.0;
        order->timestamp_sent = msg->timestamp_sent;
        order->timestamp_received = msg->timestamp_received;
        return order;
    }

    MarketOrderPtr createMarketOrder(MarketOrderMessagePtr msg)
    {
        MarketOrderPtr order = std::make_shared<MarketOrder>(++order_id_);
        order->sender_id = msg->sender_id;
        order->client_order_id = msg->client_order_id;
        order->ticker = msg->ticker;
        order->side = msg->side;
        order->status = Order::Status::NEW;
        order->remaining_quantity = msg->quantity;
        order->cumulative_quantity = 0;
        order->avg_price = 0.0;
        order->timestamp_sent = msg->timestamp_sent;
        order->timestamp_received = msg->timestamp_received;
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