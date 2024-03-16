#ifndef TRADER_ZIC_HPP
#define TRADER_ZIC_HPP

#include "traderagent.hpp"

/** Prototype ZIC trader implementation. */
class TraderZIC : public TraderAgent
{
public:

    TraderZIC(
        asio::io_context& io_context, 
        int agent_id, 
        unsigned int port, 
        std::string_view exchange, 
        std::string_view ticker, 
        Order::Side trader_side,
        double limit_price
    ) : TraderAgent(io_context, agent_id, port),
        exchange_{exchange},
        ticker_{ticker},
        trader_side_{trader_side},
        limit_price_{limit_price}
    {
    }

    void onTradingStart() override
    {
        std::cout << "Trading window started.\n";
        is_trading_ = true;
        double price = getRandomPrice();
        if (is_trading_) placeLimitOrder(exchange_, trader_side_, ticker_, 100, price);
        std::cout << ">> " << (trader_side_ == Order::Side::BID ? "BID" : "ASK") << " " << 100 << " @ " << price << "\n";
    }

    void onTradingEnd() override
    {
        is_trading_ = false;
        std::cout << "Trading window ended.\n";
    }

    void onMarketData(std::string_view exchange, MarketDataMessagePtr msg) override
    {
        double price = getRandomPrice();
        if (is_trading_) placeLimitOrder(exchange_, trader_side_, ticker_, 100, price);
        std::cout << ">> " << (trader_side_ == Order::Side::BID ? "BID" : "ASK") << " " << 100 << " @ " << price << "\n";
    }

    void onExecutionReport(std::string_view exchange, ExecutionReportMessagePtr msg) override
    {
        std::cout << "Received execution report from " << exchange << ": Order: " << msg->order_id << " Status: " << msg->status << "\n";
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