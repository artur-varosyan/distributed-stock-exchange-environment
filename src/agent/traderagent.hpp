#ifndef TRADER_AGENT_HPP
#define TRADER_AGENT_HPP

#include <iostream>

#include "agent.hpp"

class TraderAgent : public Agent
{
public:
    typedef std::string exchange_id;

    TraderAgent() = delete;
    virtual ~TraderAgent() = default;

    TraderAgent(asio::io_context& io_context)
    : Agent(io_context)
    {
    }

    /** The callback function called when new market data update is received. 
     * Derived classes must implement this. */
    virtual void onMarketData(exchange_id exchange, Message msg) = 0;

    /** The callback function called when the order acknowledgement message is received. 
     * Derived classes must implement this. */
    virtual void onOrderAck(exchange_id exchange, Message msg) = 0;



    /** Subscribes to updates for the stock with the given ticker at the given exchange. */
    void subscribeToMarket(exchange_id exchange, std::string_view ticker);

    /** Places a limit order for the given ticker at the given exchange. */
    void placeLimitOrder(exchange_id exchange, std::string_view ticker, int quantity, double price);

    /** Places a market order for the given ticker at the given exchange. */
    void placeMarketOrder(exchange_id exchange, std::string_view ticker, int quantity);

    /** Cancels the order with the given id at the given exchange. */
    void cancelOrder(exchange_id exchange, int order_id);
    
};

#endif