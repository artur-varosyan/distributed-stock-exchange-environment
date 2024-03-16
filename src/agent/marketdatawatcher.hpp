#ifndef MARKET_DATA_WATCHER_HPP
#define MARKET_DATA_WATCHER_HPP

#include "traderagent.hpp"

class MarketDataWatcher : public TraderAgent
{
public:

    MarketDataWatcher(asio::io_context& io_context, int agent_id, unsigned int port)
    : TraderAgent(io_context, agent_id, port)
    {
    };

    MarketDataWatcher(asio::io_context& io_context, int agent_id)
    : TraderAgent(io_context, agent_id)
    {
    };

    void onTradingStart() override
    {
        std::cout << "Trading window started.\n";
    }

    void onTradingEnd() override
    {
        std::cout << "Trading window ended.\n";
    }

    void onMarketData(std::string_view exchange, MarketDataMessagePtr msg) override
    {
        // Clear the screen
        std::cout << "\033[2J\033[1;1H";
        // Print the market data
        std::cout << exchange << ":\n" << msg->summary << "\n";
    }

    void onOrderAck(std::string_view exchange, OrderAckMessagePtr msg) override
    {
        throw std::runtime_error("MarketDataWatcher does not place order and cannot receive order ack.");
    }

};

#endif