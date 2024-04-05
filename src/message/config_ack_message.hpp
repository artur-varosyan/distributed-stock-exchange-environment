#ifndef CONFIG_ACK_MESSAGE_HPP
#define CONFIG_ACK_MESSAGE_HPP

#include "message.hpp"

/** Acknowledgement that a node was configured successfully. */
class ConfigAckMessage : public Message
{
public:

    ConfigAckMessage() : Message(MessageType::CONFIG) {};

private:

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & boost::serialization::base_object<Message>(*this);
    }

};

typedef std::shared_ptr<ConfigAckMessage> ConfigAckMessagePtr;

#endif