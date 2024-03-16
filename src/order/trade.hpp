#ifndef TRADE_HPP
#define TRADE_HPP

#include <boost/serialization/serialization.hpp>

#include <string>
#include <chrono>

#include "order.hpp"

struct Trade : std::enable_shared_from_this<Trade>
{
public: 

    Trade()
    {
        std::chrono::system_clock::duration now = std::chrono::system_clock::now().time_since_epoch();
        timestamp = std::chrono::duration_cast<std::chrono::nanoseconds>(now).count();
    }

    static std::shared_ptr<Trade> createFromOrders(OrderPtr resting_order, OrderPtr aggressing_order, int id)
    {
        std::shared_ptr<Trade> trade = std::make_shared<Trade>();
        trade->id = id;
        trade->ticker = resting_order->ticker;
        trade->quantity = std::min(resting_order->quantity, aggressing_order->quantity);
        trade->price = resting_order->price;
        if (aggressing_order->side == Order::Side::BID)
        {
            trade->buyer_id = aggressing_order->sender_id;
            trade->seller_id = resting_order->sender_id;
        }
        else
        {
            trade->buyer_id = resting_order->sender_id;
            trade->seller_id = aggressing_order->sender_id;
        }
        trade->aggressing_order_id = aggressing_order->id;
        trade->resting_order_id = resting_order->id;
        return trade;
    }

    int id;
    std::string ticker;
    int quantity;
    double price;
    unsigned long long timestamp = 0;
    int buyer_id;
    int seller_id;
    int aggressing_order_id;
    int resting_order_id;

private:

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & id;
        ar & ticker;
        ar & quantity;
        ar & price;
        ar & timestamp;
        ar & buyer_id;
        ar & seller_id;
        ar & aggressing_order_id;
        ar & resting_order_id;
    }

    friend std::ostream& operator<<(std::ostream& os, const Trade& trade)
    {
        os << trade.timestamp << " [Trade] Id: " << trade.id << " " << trade.ticker << " " << trade.quantity << " @ $" << trade.price << " Buyer: " << trade.buyer_id << " Seller: " << trade.seller_id;
        return os;
    }
};

typedef std::shared_ptr<Trade> TradePtr;

#endif