#ifndef TRADE_FACTORY_HPP
#define TRADE_FACTORY_HPP

#include "trade.hpp"
#include "../order/limitorder.hpp"
#include "../order/marketorder.hpp"

class TradeFactory
{
public:

    TradeFactory() = default;

    TradePtr createFromLimitOrders(LimitOrderPtr resting_order, LimitOrderPtr aggressing_order)
    {
        std::shared_ptr<Trade> trade = std::make_shared<Trade>();
        trade->id = ++trade_id_;
        trade->ticker = resting_order->ticker;
        trade->quantity = std::min(resting_order->remaining_quantity, aggressing_order->remaining_quantity);
        trade->price = resting_order->price;
        if (aggressing_order->side == Order::Side::BID)
        {
            trade->buyer_id = aggressing_order->sender_id;
            trade->seller_id = resting_order->sender_id;
            trade->buyer_priv_value = aggressing_order->priv_value;
            trade->seller_priv_value = resting_order->priv_value;
        }
        else
        {
            trade->buyer_id = resting_order->sender_id;
            trade->seller_id = aggressing_order->sender_id;
            trade->buyer_priv_value = resting_order->priv_value;
            trade->seller_priv_value = aggressing_order->priv_value;
        }
        trade->aggressing_order_id = aggressing_order->id;
        trade->resting_order_id = resting_order->id;

        volume_traded_ += trade->quantity;

        return trade;
    }

    TradePtr createFromLimitAndMarketOrders(LimitOrderPtr resting_order, MarketOrderPtr aggressing_order)
    {
        std::shared_ptr<Trade> trade = std::make_shared<Trade>();
        trade->id = ++trade_id_;
        trade->ticker = resting_order->ticker;
        trade->quantity = std::min(resting_order->remaining_quantity, aggressing_order->remaining_quantity);
        trade->price = resting_order->price;
        if (aggressing_order->side == Order::Side::BID)
        {
            trade->buyer_id = aggressing_order->sender_id;
            trade->seller_id = resting_order->sender_id;
            trade->buyer_priv_value = aggressing_order->priv_value;
            trade->seller_priv_value = resting_order->priv_value;
        }
        else
        {
            trade->buyer_id = resting_order->sender_id;
            trade->seller_id = aggressing_order->sender_id;
            trade->buyer_priv_value = resting_order->priv_value;
            trade->seller_priv_value = aggressing_order->priv_value;
        }
        trade->aggressing_order_id = aggressing_order->id;
        trade->resting_order_id = resting_order->id;

        volume_traded_ += trade->quantity;

        return trade;
    }

    int getNumberOfTrades() const
    {
        return trade_id_;
    }

private:

    int trade_id_ = 0;
    int volume_traded_ = 0;
};

#endif