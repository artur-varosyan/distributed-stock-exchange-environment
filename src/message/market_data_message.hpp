#ifndef MARKET_DATA_MESSAGE_HPP
#define MARKET_DATA_MESSAGE_HPP

#include "message.hpp"
#include "messagetype.hpp"

struct MarketDataMessage : public Message
{
    MarketDataMessage() : Message(MessageType::MARKET_DATA) {};

    std::string ticker;
    double price;

private:

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & boost::serialization::base_object<Message>(*this);
        ar & ticker;
        ar & price;
    }

};

#endif
