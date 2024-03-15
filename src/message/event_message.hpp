#ifndef SIMULATION_EVENT_HPP
#define SIMULATION_EVENT_HPP

#include "message.hpp"

struct EventMessage : public Message
{
public:

    enum class EventType : int 
    {
        TRADING_SESSION_START,
        TRADING_SESSION_END,
        BREAK_START,
        BREAK_END,
    };

    EventMessage() : Message(MessageType::EVENT) {};
    
    EventMessage(EventType type) 
    : Message(MessageType::EVENT), 
      event_type{type} 
    {};

    EventType event_type;

private:

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & boost::serialization::base_object<Message>(*this);
        ar & event_type;
    }

};

typedef std::shared_ptr<EventMessage> EventMessagePtr;

#endif