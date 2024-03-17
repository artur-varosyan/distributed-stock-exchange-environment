#ifndef REJECT_CANCEL_MESSAGE_HPP
#define REJECT_CANCEL_MESSAGE_HPP

#include "message.hpp"

class CancelRejectMessage : public Message
{
public:

    CancelRejectMessage() : Message(MessageType::CANCEL_REJECT) {};

    int order_id;

private:

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & boost::serialization::base_object<Message>(*this);
        ar & order_id;
    }

};

typedef std::shared_ptr<CancelRejectMessage> CancelRejectMessagePtr;

#endif