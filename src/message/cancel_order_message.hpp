#ifndef CANCEL_ORDER_MESSAGE_HPP
#define CANCEL_ORDER_MESSAGE_HPP

#include "message.hpp"
#include "messagetype.hpp"

class CancelOrderMessage : public Message
{
public:

    CancelOrderMessage() : Message(MessageType::CANCEL_ORDER) {};

    int order_id;
    Order::Side side;

private:

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & boost::serialization::base_object<Message>(*this);
        ar & order_id;
        ar & side;
    }

};

typedef std::shared_ptr<CancelOrderMessage> CancelOrderMessagePtr;

#endif