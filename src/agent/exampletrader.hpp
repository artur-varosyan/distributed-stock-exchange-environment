#ifndef EXAMPLE_TRADER_HPP
#define EXAMPLE_TRADER_HPP

#include <iostream>
#include <boost/asio.hpp>

#include "traderagent.hpp"

class ExampleTrader : public TraderAgent
{
public:

    ExampleTrader(asio::io_context& io_context, int agent_id, unsigned int port)
    : TraderAgent(io_context, agent_id, port)
    {
    }

    ExampleTrader(asio::io_context& io_context, int agent_id)
    : TraderAgent(io_context, agent_id)
    {
    }

    void onTradingStart() override
    {
        std::cout << "Trading session started.\n";
        is_trading_ = true;
        placeRandomOrder();
    }

    void onTradingEnd() override
    {
        is_trading_ = false;
        std::cout << "Trading session ended.\n";
    }

    void onMarketData(std::string_view exchange, MarketDataMessagePtr msg) override
    {
        // std::cout << "Received market data from " << exchange << ":\n" << msg->summary << "\n";
        if (is_trading_) placeRandomOrder();
    }

    void onExecutionReport(std::string_view exchange, ExecutionReportMessagePtr msg) override
    {
        std::cout << "Received execution report from " << exchange << ": Order: " << msg->order_id << " Status: " << msg->status << "\n";
    }

    void placeRandomOrder()
    {
        // Place a bid or ask at random
        Order::Side side = (rand() % 2 == 0) ? Order::Side::BID : Order::Side::ASK;
        // Choose a random price between 90 and 110
        double price = 90 + (rand() % 21);
        // Choose random quantity between 1 and 100
        int quantity = 1 + (rand() % 100);

        std::cout << ">> " << (side == Order::Side::BID ? "BID" : "ASK") << " " << quantity << " @ " << price << "\n";
        placeLimitOrder("LSE", side, "AAPL", quantity, price);
    }

private:

    bool is_trading_ = false;
};

#endif