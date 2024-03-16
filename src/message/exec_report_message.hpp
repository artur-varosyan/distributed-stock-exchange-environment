#ifndef ORDER_ACK_MESSAGE_HPP
#define ORDER_ACK_MESSAGE_HPP

#include <optional>
#include <boost/serialization/optional.hpp>

#include "message.hpp"
#include "messagetype.hpp"
#include "../order/order.hpp"
#include "../order/trade.hpp"

class ExecutionReportMessage : public Message
{
public:

    ExecutionReportMessage() : Message(MessageType::EXECUTION_REPORT) {};

    /** Creates an ExecutionReport message from a new or cancelled order. */
    static std::shared_ptr<ExecutionReportMessage> createFromOrder(OrderPtr order, bool cancelled = false)
    {
        std::shared_ptr<ExecutionReportMessage> message = std::make_shared<ExecutionReportMessage>();
        message->order_id = order->id;
        message->ticker = order->ticker;
        message->status = cancelled ? Order::Status::CANCELLED : Order::Status::NEW;
        message->side = order->side;
        message->price = order->price;
        message->remaining_quantity = order->remaining_quantity;
        message->cumulative_quantity = order->cumulative_quantity;
        message->avg_price = order->avg_price;
        message->trade_price = std::nullopt;
        message->trade_quantity = std::nullopt;
        return message;
    };

    /** Creates an ExecutionReport message from the state of a post-trade order and the trade object. */
    static std::shared_ptr<ExecutionReportMessage> createFromTrade(OrderPtr order, TradePtr trade)
    {
        Order::Status status = order->remaining_quantity == 0 ? Order::Status::FILLED : Order::Status::PARTIALLY_FILLED;
        std::shared_ptr<ExecutionReportMessage> message = std::make_shared<ExecutionReportMessage>();
        message->order_id = order->id;
        message->ticker = order->ticker;
        message->status = status;
        message->side = order->side;
        message->price = order->price;
        message->remaining_quantity = order->remaining_quantity;
        message->cumulative_quantity = order->cumulative_quantity;
        message->avg_price = order->avg_price;
        message->trade_price = trade->price;
        message->trade_quantity = trade->quantity;
        return message;
    };

    int order_id;
    std::string ticker;
    Order::Status status;
    Order::Side side;
    double price;                          /** The price at which the order was submitted. */
    int remaining_quantity;                /** The remaining quantity of the order to be fulfilled. */
    int cumulative_quantity;               /** The quantity of the order that has been fulfilled already. */

    // Only present in the case of a filled or partially filled order.

    std::optional<double> avg_price;       /** The average price at which the order has been fulfilled. */
    std::optional<double> trade_price;     /** The price at which the last fill was fulfilled. */
    std::optional<int> trade_quantity;     /** The quantity of the order that was last fulfilled. */

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
        ar & price;
        ar & remaining_quantity;
        ar & cumulative_quantity;
        ar & trade_price;
        ar & trade_quantity;
    }

};

typedef std::shared_ptr<ExecutionReportMessage> ExecutionReportMessagePtr;

#endif