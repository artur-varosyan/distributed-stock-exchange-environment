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

    void onMarketData(std::string_view exchange, MarketDataMessagePtr msg) override
    {
        std::cout << "Received market data from " << exchange << ":\n" << msg->summary << "\n";
    }

    void onOrderAck(std::string_view exchange, OrderAckMessagePtr msg) override
    {
        std::cout << "Received order ack from " << exchange << ": " << msg->order_id << " " << msg->success << "\n";
    }

};

#endif