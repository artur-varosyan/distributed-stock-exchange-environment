#include <iostream>

#include "order.hpp"
#include "orderbook.hpp"

#include <boost/serialization/export.hpp>

BOOST_CLASS_EXPORT(OrderBook::Summary);

void OrderBook::addOrder(OrderPtr order)
{
    if (order->side == Order::Side::BID)
    {
        bids_.push(order);
        bid_size_ += order->quantity;
    }
    else
    {
        asks_.push(order);
        ask_size_ += order->quantity;
    }
}

bool OrderBook::removeOrder(int order_id, Order::Side side)
{
    if (side == Order::Side::BID)
    {
        std::optional<OrderPtr> order = bids_.remove(order_id);
        if (order.has_value())
        {
            bid_size_ -= order.value()->quantity;
            return true;
        }
    }
    else
    {
        std::optional<OrderPtr> order = asks_.remove(order_id);
        if (order.has_value())
        {
            ask_size_ -= order.value()->quantity;
            return true;
        }
    }
    return false;
}

std::optional<OrderPtr> OrderBook::bestBid()
{
    if (bids_.empty())
    {
        return std::nullopt;
    }
    return bids_.top();
}

std::optional<OrderPtr> OrderBook::bestAsk()
{
    if (asks_.empty())
    {
        return std::nullopt;
    }
    return asks_.top();
}

void OrderBook::popBestBid()
{
    if (!bids_.empty())
    {
        bid_size_ -= bids_.top()->quantity;
        bids_.pop();
    }
}

void OrderBook::popBestAsk()
{
    if (!asks_.empty())
    {
        ask_size_ -= asks_.top()->quantity;
        asks_.pop();
    }
}

OrderBook::Summary OrderBook::getSummary()
{
    Summary summary;
    summary.ticker = ticker_;
    summary.best_bid = bestBid().has_value() ? bestBid().value()->price : 0;
    summary.best_ask = bestAsk().has_value() ? bestAsk().value()->price : 0;
    summary.bid_size = bid_size_;
    summary.ask_size = ask_size_;
    summary.timestamp = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    return summary;
}