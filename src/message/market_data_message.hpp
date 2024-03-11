#ifndef MARKET_DATA_MESSAGE_HPP
#define MARKET_DATA_MESSAGE_HPP

#include "message.hpp"
#include "messagetype.hpp"
#include "../order/orderbook.hpp"

struct MarketDataMessage : public Message
{
    MarketDataMessage() : Message(MessageType::MARKET_DATA) {};

    OrderBook::Summary summary;

private:

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & boost::serialization::base_object<Message>(*this);
        ar & summary;
    }

};

typedef std::shared_ptr<MarketDataMessage> MarketDataMessagePtr;

#endif
