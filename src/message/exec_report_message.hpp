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
    static std::shared_ptr<ExecutionReportMessage> createFromOrder(OrderPtr order, Order::Status status)
    {
        double price;
        if (order->type == Order::Type::LIMIT)
        {
            price = std::dynamic_pointer_cast<LimitOrder>(order)->price;
        }
        else
        {
            price = 0;
        }

        std::shared_ptr<ExecutionReportMessage> message = std::make_shared<ExecutionReportMessage>();
        message->order_id = order->id;
        message->ticker = order->ticker;
        message->status = status;
        message->side = order->side;
        message->price = price;
        message->remaining_quantity = order->remaining_quantity;
        message->cumulative_quantity = order->cumulative_quantity;
        message->avg_price = 0;
        message->trade_price = 0;
        message->trade_quantity = 0;
        return message;
    };

    /** Creates an ExecutionReport message from the state of a post-trade order and the trade object. */
    static std::shared_ptr<ExecutionReportMessage> createFromTrade(OrderPtr order, TradePtr trade)
    {
        Order::Status status = order->remaining_quantity == 0 ? Order::Status::FILLED : Order::Status::PARTIALLY_FILLED;
        double price;
        if (order->type == Order::Type::LIMIT)
        {
            price = std::dynamic_pointer_cast<LimitOrder>(order)->price;
        }
        else
        {
            price = trade->price;
        }

        std::shared_ptr<ExecutionReportMessage> message = std::make_shared<ExecutionReportMessage>();
        message->order_id = order->id;
        message->ticker = order->ticker;
        message->status = status;
        message->side = order->side;
        message->remaining_quantity = order->remaining_quantity;
        message->cumulative_quantity = order->cumulative_quantity;
        message->price = price;
        message->avg_price = order->avg_price;
        message->trade_price = trade->price;
        message->trade_quantity = trade->quantity;
        return message;
    };

    int order_id;
    std::string ticker;
    Order::Status status;
    Order::Side side;
    Order::Type type;
    double price;                          /** The price at which the order was placed. (only limit orders) */
    int remaining_quantity;                /** The remaining quantity of the order to be fulfilled. */
    int cumulative_quantity;               /** The quantity of the order that has been fulfilled already. */

    // Only present in the case of a filled or partially filled order.

    double avg_price;       /** The average price at which the order has been fulfilled. */
    double trade_price;     /** The price at which the last fill was fulfilled. */
    int trade_quantity;     /** The quantity of the order that was last fulfilled. */

private:
    
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & boost::serialization::base_object<Message>(*this);
        ar & order_id;
        ar & ticker;
        ar & status;
        ar & side;
        ar & type;
        ar & price;
        ar & remaining_quantity;
        ar & cumulative_quantity;
        ar & trade_price;
        ar & trade_quantity;
    }

};

typedef std::shared_ptr<ExecutionReportMessage> ExecutionReportMessagePtr;

#endif