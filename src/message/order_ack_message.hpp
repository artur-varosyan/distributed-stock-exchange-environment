#ifndef ORDER_ACK_MESSAGE_HPP
#define ORDER_ACK_MESSAGE_HPP

#include "message.hpp"
#include "messagetype.hpp"

struct OrderAckMessage : public Message
{
    OrderAckMessage() : Message(MessageType::ORDER_ACK) {};

    int order_id;
    bool success;

private:
    
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & boost::serialization::base_object<Message>(*this);
        ar & order_id;
        ar & success;
    }

};

typedef std::shared_ptr<OrderAckMessage> OrderAckMessagePtr;

#endif