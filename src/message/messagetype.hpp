#ifndef MESSAGE_TYPE_HPP
#define MESSAGE_TYPE_HPP

enum struct MessageType
{
    INIT,
    CONFIG,
    LOGON,
    MARKET_DATA,
    LIMIT_ORDER,
    MARKET_ORDER,
    CANCEL_ORDER,
    ACK,
};

#endif