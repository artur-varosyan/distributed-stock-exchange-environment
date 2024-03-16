#ifndef TRADE_HPP
#define TRADE_HPP

#include <string>
#include <chrono>

#include <boost/serialization/serialization.hpp>

class TradeFactory;

class Trade : std::enable_shared_from_this<Trade>
{
public: 

    Trade()
    {
        std::chrono::system_clock::duration now = std::chrono::system_clock::now().time_since_epoch();
        timestamp = std::chrono::duration_cast<std::chrono::nanoseconds>(now).count();
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