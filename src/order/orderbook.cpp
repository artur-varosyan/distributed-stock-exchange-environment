#include <iostream>

#include "order.hpp"
#include "orderbook.hpp"

#include <boost/serialization/export.hpp>

BOOST_CLASS_EXPORT(MarketData);

void OrderBook::addOrder(LimitOrderPtr order)
{
    if (order->side == Order::Side::BID)
    {
        bids_.push(order);
        bids_volume_ += order->remaining_quantity;
        bids_sizes_[order->price] += order->remaining_quantity;
    }
    else
    {
        asks_.push(order);
        asks_volume_ += order->remaining_quantity;
        asks_sizes_[order->price] += order->remaining_quantity;
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
            bids_sizes_[order.value()->price] -= order.value()->remaining_quantity;
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
            asks_sizes_[order.value()->price] -= order.value()->remaining_quantity;
            --order_count_;
        }
        return order;
    }
}

void OrderBook::updateOrderWithTrade(OrderPtr order, TradePtr trade)
    {
        // Update average price executed
        order->avg_price = ((order->cumulative_quantity * order->avg_price) + (trade->quantity * trade->price)) / (order->cumulative_quantity + trade->quantity);

        // Update quantities
        order->cumulative_quantity += trade->quantity;
        order->remaining_quantity -= trade->quantity;

        // Update order status
        if (order->remaining_quantity == 0)
        {
            order->status = Order::Status::FILLED;
        }
        else
        {
            order->status = Order::Status::PARTIALLY_FILLED;
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

int OrderBook::bestBidSize()
{
    std::optional<LimitOrderPtr> best_bid = bestBid();
    if (best_bid.has_value())
    {
        return bids_sizes_.at(best_bid.value()->price);
    }
    else 
    {
        return 0;
    }
}

std::optional<LimitOrderPtr> OrderBook::bestAsk()
{
    if (asks_.empty())
    {
        return std::nullopt;
    }
    return asks_.top();
}

int OrderBook::bestAskSize()
{
    std::optional<LimitOrderPtr> best_ask = bestAsk();
    if (best_ask.has_value())
    {
        return asks_sizes_.at(best_ask.value()->price);
    }
    else 
    {
        return 0;
    }
}

void OrderBook::popBestBid()
{
    if (!bids_.empty())
    {
        bids_volume_ -= bids_.top()->remaining_quantity;
        bids_sizes_[bids_.top()->price] -= bids_.top()->remaining_quantity;
        bids_.pop();
        --order_count_;
    }
}

void OrderBook::popBestAsk()
{
    if (!asks_.empty())
    {
        asks_volume_ -= asks_.top()->remaining_quantity;
        asks_sizes_[asks_.top()->price] -= asks_.top()->remaining_quantity;
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

MarketData OrderBook::getLiveMarketData()
{
    MarketData data;
    data.ticker = ticker_;
    data.best_bid = bestBid().has_value() ? bestBid().value()->price : -1;
    data.best_ask = bestAsk().has_value() ? bestAsk().value()->price : -1;
    data.best_bid_size = bestBidSize();
    data.best_ask_size =  bestAskSize();

    data.asks_volume = asks_volume_;
    data.bids_volume = bids_volume_;
    data.asks_count = asks_.size();
    data.bids_count = bids_.size();

    data.last_price_traded = last_trade_.has_value() ? last_trade_.value()->price : -1;
    data.last_quantity_traded = last_trade_.has_value() ? last_trade_.value()->quantity : 0;

    data.high_price = trade_high_.has_value() ? trade_high_.value() : -1;
    data.low_price = trade_low_.has_value() ? trade_low_.value() : -1;
    data.cumulative_volume_traded = trade_volume_;
    data.trades_count = trade_count_;

    data.timestamp = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    return data;
}