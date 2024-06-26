#ifndef SUBSCRIBE_MESSAGE_HPP
#define SUBSCRIBE_MESSAGE_HPP

#include "message.hpp"
#include "messagetype.hpp"

class SubscribeMessage : public Message
{
public:

    SubscribeMessage() : Message(MessageType::SUBSCRIBE) {};

    std::string ticker;
    std::string address;

private:

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & boost::serialization::base_object<Message>(*this);
        ar & ticker;
        ar & address;
    }

};

typedef std::shared_ptr<SubscribeMessage> SubscribeMessagePtr;

#endif