#ifndef MARKET_ORDER_MESSAGE_HPP
#define MARKET_ORDER_MESSAGE_HPP

#include "message.hpp"
#include "messagetype.hpp"

struct MarketOrderMessage : public Message
{
    MarketOrderMessage() : Message(MessageType::MARKET_ORDER) {};

    std::string ticker;
    Order::Side side;
    int quantity;

private:

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & boost::serialization::base_object<Message>(*this);
        ar & ticker;
        ar & side;
        ar & quantity;
    }

};

typedef std::shared_ptr<MarketOrderMessage> MarketOrderMessagePtr;

#endif