#ifndef LIMIT_ORDER_MESSAGE_HPP
#define LIMIT_ORDER_MESSAGE_HPP

#include "message.hpp"
#include "messagetype.hpp"
#include "../order/order.hpp"

class LimitOrderMessage : public Message
{
public:

    LimitOrderMessage() : Message(MessageType::LIMIT_ORDER) {};

    std::string ticker;
    Order::Side side;
    int quantity;
    double price;
    int separator;

private:

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & boost::serialization::base_object<Message>(*this);
        ar & ticker;
        ar & side;
        ar & quantity;
        ar & price;
        ar & separator;
    }

};

typedef std::shared_ptr<LimitOrderMessage> LimitOrderMessagePtr;

#endif