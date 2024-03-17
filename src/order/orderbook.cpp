#include <iostream>

#include "order.hpp"
#include "orderbook.hpp"

#include <boost/serialization/export.hpp>

BOOST_CLASS_EXPORT(OrderBook::Summary);

void OrderBook::addOrder(LimitOrderPtr order)
{
    if (order->side == Order::Side::BID)
    {
        bids_.push(order);
        bids_volume_ += order->remaining_quantity;
    }
    else
    {
        asks_.push(order);
        asks_volume_ += order->remaining_quantity;
    }
    ++order_count_;
}

std::optional<LimitOrderPtr> OrderBook::removeOrder(int order_id, Order::Side side)
{
    if (side == Order::Side::BID)
    {
        std::optional<LimitOrderPtr> order = bids_.remove(order_id);
        if (order.has_value())
        {
            bids_volume_ -= order.value()->remaining_quantity;
            --order_count_;
        }
        return order;
    }
    else
    {
        std::optional<LimitOrderPtr> order = asks_.remove(order_id);
        if (order.has_value())
        {
            asks_volume_ -= order.value()->remaining_quantity;
            --order_count_;
        }
        return order;
    }
}

std::optional<LimitOrderPtr> OrderBook::bestBid()
{
    if (bids_.empty())
    {
        return std::nullopt;
    }
    return bids_.top();
}

std::optional<LimitOrderPtr> OrderBook::bestAsk()
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
        bids_volume_ -= bids_.top()->remaining_quantity;
        bids_.pop();
        --order_count_;
    }
}

void OrderBook::popBestAsk()
{
    if (!asks_.empty())
    {
        asks_volume_ -= asks_.top()->remaining_quantity;
        asks_.pop();
        --order_count_;
    }
}

bool OrderBook::contains(int order_id, Order::Side side)
{
    if (side == Order::Side::BID)
    {
        return bids_.find(order_id).has_value();
    }
    else
    {
        return asks_.find(order_id).has_value();
    }
}

void OrderBook::logTrade(TradePtr trade)
{
    last_trade_ = trade;
    trade_high_ = trade_high_.has_value() ? std::max(trade_high_.value(), trade->price) : trade->price;
    trade_low_ = trade_low_.has_value() ? std::min(trade_low_.value(), trade->price) : trade->price;
    trade_volume_ += trade->quantity;
    ++trade_count_;
}

OrderBook::Summary OrderBook::getSummary()
{
    Summary summary;
    summary.ticker = ticker_;
    summary.best_bid = bestBid().has_value() ? bestBid().value()->price : -1;
    summary.best_ask = bestAsk().has_value() ? bestAsk().value()->price : -1;
    summary.best_bid_size = bestBid().has_value() ? bestBid().value()->remaining_quantity : 0;
    summary.best_ask_size =  bestBid().has_value() ? bestBid().value()->remaining_quantity : 0;

    summary.asks_volume = asks_volume_;
    summary.bids_volume = bids_volume_;
    summary.asks_count = asks_.size();
    summary.bids_count = bids_.size();

    summary.last_price_traded = last_trade_.has_value() ? last_trade_.value()->price : -1;
    summary.last_quantity_traded = last_trade_.has_value() ? last_trade_.value()->quantity : 0;

    summary.high_price = trade_high_.has_value() ? trade_high_.value() : -1;
    summary.low_price = trade_low_.has_value() ? trade_low_.value() : -1;
    summary.cumulative_volume_traded = trade_volume_;
    summary.trades_count = trade_count_;

    summary.timestamp = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    return summary;
}