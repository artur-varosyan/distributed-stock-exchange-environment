#ifndef CANCEL_ORDER_MESSAGE_HPP
#define CANCEL_ORDER_MESSAGE_HPP

#include "message.hpp"
#include "messagetype.hpp"

struct CancelOrderMessage : public Message
{
    CancelOrderMessage() : Message(MessageType::CANCEL_ORDER) {};

    std::string order_id;

private:

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & boost::serialization::base_object<Message>(*this);
        ar & order_id;
    }

};

typedef std::shared_ptr<CancelOrderMessage> CancelOrderMessagePtr;

#endif