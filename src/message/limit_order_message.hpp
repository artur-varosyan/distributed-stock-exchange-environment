#ifndef LIMIT_ORDER_MESSAGE_HPP
#define LIMIT_ORDER_MESSAGE_HPP

#include "message.hpp"
#include "messagetype.hpp"
#include "../order/order.hpp"

class LimitOrderMessage : public Message
{
public:

    LimitOrderMessage() : Message(MessageType::LIMIT_ORDER) {};

    int client_order_id;
    std::string ticker;
    Order::Side side;
    Order::TimeInForce time_in_force;
    int quantity;
    double price;
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
        ar & time_in_force;
        ar & quantity;
        ar & price;
        ar & priv_value;
    }

};

typedef std::shared_ptr<LimitOrderMessage> LimitOrderMessagePtr;

#endif