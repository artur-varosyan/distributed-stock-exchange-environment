#ifndef MESSAGE_TYPE_HPP
#define MESSAGE_TYPE_HPP

enum struct MessageType
{
    INIT,
    CONFIG,
    MARKET_DATA,
    LIMIT_ORDER,
    MARKET_ORDER,
    CANCEL_ORDER,
    ORDER_ACK,
};

#endif