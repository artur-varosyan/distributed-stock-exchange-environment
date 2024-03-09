#ifndef SUBSCRIBE_MESSAGE_HPP
#define SUBSCRIBE_MESSAGE_HPP

#include "message.hpp"
#include "messagetype.hpp"

struct SubscribeMessage : public Message
{
    SubscribeMessage() : Message(MessageType::SUBSCRIBE) {};

    std::string ticker;

private:

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & boost::serialization::base_object<Message>(*this);
        ar & ticker;
    }

};

#endif