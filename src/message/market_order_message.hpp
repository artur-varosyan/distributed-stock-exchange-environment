#ifndef MARKET_ORDER_MESSAGE_HPP
#define MARKET_ORDER_MESSAGE_HPP

#include "message.hpp"
#include "messagetype.hpp"

class MarketOrderMessage : public Message
{
public:

    MarketOrderMessage() : Message(MessageType::MARKET_ORDER) {};

    int client_order_id;
    std::string ticker;
    Order::Side side;
    int quantity;
    double priv_value;

private:

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & boost::serialization::base_object<Message>(*this);
        ar & client_order_id;
        ar & ticker;
        ar & side;
        ar & quantity;
        ar & priv_value;
    }

};

typedef std::shared_ptr<MarketOrderMessage> MarketOrderMessagePtr;

#endif