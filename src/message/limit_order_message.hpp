#ifndef LIMIT_ORDER_MESSAGE_HPP
#define LIMIT_ORDER_MESSAGE_HPP

#include "message.hpp"
#include "messagetype.hpp"
#include "../order/order.hpp"

struct LimitOrderMessage : public Message
{
    LimitOrderMessage() : Message(MessageType::LIMIT_ORDER) {};

    std::string ticker;
    Order::Side side;
    double price;
    int quantity;

private:

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & boost::serialization::base_object<Message>(*this);
        ar & ticker;
        ar & side;
        ar & price;
        ar & quantity;
    }

};

typedef std::shared_ptr<LimitOrderMessage> LimitOrderMessagePtr;

#endif