#ifndef MESSAGE_TYPE_HPP
#define MESSAGE_TYPE_HPP

enum class MessageType : int
{
    INIT,
    CONFIG,
    EVENT,
    MARKET_DATA,
    SUBSCRIBE,
    LIMIT_ORDER,
    MARKET_ORDER,
    CANCEL_ORDER,
    EXECUTION_REPORT
};

#endif