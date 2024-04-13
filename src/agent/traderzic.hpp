#ifndef TRADER_ZIC_HPP
#define TRADER_ZIC_HPP

#include "traderagent.hpp"

/** Prototype ZIC trader implementation. */
class TraderZIC : public TraderAgent
{
public:

    TraderZIC(NetworkEntity *network_entity, TraderConfigPtr config)
    : TraderAgent(network_entity, config),
      exchange_{config->exchange_name},
      ticker_{config->ticker},
      trader_side_{config->side},
      limit_price_{config->limit}
    {
    }

    void onTradingStart() override
    {
        std::cout << "Trading window started.\n";
        is_trading_ = true;
        double price = getRandomPrice();
        if (is_trading_) 
        {
            placeLimitOrder(exchange_, trader_side_, ticker_, 100, price);
            std::cout << ">> " << (trader_side_ == Order::Side::BID ? "BID" : "ASK") << " " << 100 << " @ " << price << "\n";
        }
    }

    void onTradingEnd() override
    {
        is_trading_ = false;
        std::cout << "Trading window ended.\n";
    }

    void onMarketData(std::string_view exchange, MarketDataMessagePtr msg) override
    {
        int quantity = 100;
        double price = getRandomPrice();
        if (is_trading_) 
        {
            placeLimitOrder(exchange_, trader_side_, ticker_, quantity, price);
            std::cout << ">> " << (trader_side_ == Order::Side::BID ? "BID" : "ASK") << " " << quantity << " @ " << price << "\n";
        }
    }

    void onExecutionReport(std::string_view exchange, ExecutionReportMessagePtr msg) override
    {
        std::cout << "Received execution report from " << exchange << ": Order: " << msg->order->id << " Status: " << msg->order->status << 
        " Qty remaining = " << msg->order->remaining_quantity << "\n";
    }

    void onCancelReject(std::string_view exchange, CancelRejectMessagePtr msg) override
    {
        std::cout << "Received cancel reject from " << exchange << ": Order: " << msg->order_id;
    }

private:

    double getRandomPrice()
    {
        if (trader_side_ == Order::Side::BID)
        {
            return MIN_PRICE + (rand() % (int)(limit_price_ - MIN_PRICE + 1));
        }
        else
        {
            return limit_price_ + (rand() % (int)(MAX_PRICE - limit_price_ + 1));
        }
    }

    std::string exchange_;
    std::string ticker_;
    Order::Side trader_side_;
    double limit_price_;

    bool is_trading_ = false;

    constexpr static double MIN_PRICE = 1.0;
    constexpr static double MAX_PRICE = 200.0;
};

#endif