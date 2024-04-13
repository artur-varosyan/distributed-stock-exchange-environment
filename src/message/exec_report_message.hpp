#ifndef ORDER_ACK_MESSAGE_HPP
#define ORDER_ACK_MESSAGE_HPP

#include "message.hpp"
#include "messagetype.hpp"
#include "../order/order.hpp"
#include "../trade/trade.hpp"

class ExecutionReportMessage : public Message
{
public:

    ExecutionReportMessage() : Message(MessageType::EXECUTION_REPORT) {};

    /** Creates an ExecutionReport message from a new or cancelled order. */
    static std::shared_ptr<ExecutionReportMessage> createFromOrder(OrderPtr order)
    {
        std::shared_ptr<ExecutionReportMessage> message = std::make_shared<ExecutionReportMessage>();
        message->order = order;
        message->trade = nullptr;
        return message;
    };

    /** Creates an ExecutionReport message from the state of a post-trade order and the trade object. */
    static std::shared_ptr<ExecutionReportMessage> createFromTrade(OrderPtr order, TradePtr trade)
    {
        std::shared_ptr<ExecutionReportMessage> message = std::make_shared<ExecutionReportMessage>();
        message->order = order;
        message->trade = trade;
        return message;
    };

    OrderPtr order;
    TradePtr trade;

private:
    
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & boost::serialization::base_object<Message>(*this);
        ar & order;
        ar & trade;
    }

};

typedef std::shared_ptr<ExecutionReportMessage> ExecutionReportMessagePtr;

#endif