#ifndef MARKET_DATA_MESSAGE_HPP
#define MARKET_DATA_MESSAGE_HPP

#include "message.hpp"
#include "messagetype.hpp"
#include "../order/orderbook.hpp"
#include "../trade/marketdata.hpp"

class MarketDataMessage : public Message
{
public:

    MarketDataMessage() : Message(MessageType::MARKET_DATA) {};

    MarketDataPtr data;

private:

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & boost::serialization::base_object<Message>(*this);
        ar & data;
    }

};

typedef std::shared_ptr<MarketDataMessage> MarketDataMessagePtr;

#endif
