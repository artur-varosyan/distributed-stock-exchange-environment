#ifndef TRADER_SHVR_HPP
#define TRADER_SHVR_HPP

#include "traderagent.hpp"

class TraderShaver : public TraderAgent
{
public:

    TraderShaver(
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
    }

    void onTradingEnd() override
    {
        is_trading_ = false;
        std::cout << "Trading window ended.\n";
    }

    void onMarketData(std::string_view exchange, MarketDataMessagePtr msg) override
    {
        int quantity = 100;
        if (is_trading_) 
        {
            double price = getShaverPrice(msg);
            placeLimitOrder(exchange_, trader_side_, ticker_, quantity, price);
            std::cout << ">> " << (trader_side_ == Order::Side::BID ? "BID" : "ASK") << " " << quantity << " @ " << price << "\n";
        }
    }

    void onExecutionReport(std::string_view exchange, ExecutionReportMessagePtr msg) override
    {
        std::cout << "Received execution report from " << exchange << ": Order: " << msg->order_id << " Status: " << msg->status << 
        " Qty remaining = " << msg->remaining_quantity << "\n";
    }

    void onCancelReject(std::string_view exchange, CancelRejectMessagePtr msg) override
    {
        throw std::runtime_error("Shaver trader does not cancel orders therefore cannot receive cancel rejection.");
    }

private:

    double getShaverPrice(MarketDataMessagePtr msg)
    {
        if (trader_side_ == Order::Side::BID)
        {
            double shaved_price = msg->summary.best_bid + 1;
            return std::min(shaved_price, limit_price_);
        }
        else
        {
            double shaved_price = msg->summary.best_ask - 1;
            return std::max(shaved_price, limit_price_);
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