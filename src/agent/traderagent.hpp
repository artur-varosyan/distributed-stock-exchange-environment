#ifndef TRADER_AGENT_HPP
#define TRADER_AGENT_HPP

#include <iostream>

#include "agent.hpp"
#include "../order/order.hpp"
#include "../message/market_data_message.hpp"
#include "../message/exec_report_message.hpp"
#include "../message/subscribe_message.hpp"
#include "../message/limit_order_message.hpp"
#include "../message/market_order_message.hpp"
#include "../message/cancel_order_message.hpp"
#include "../message/event_message.hpp"

class TraderAgent : public Agent
{
public:

    TraderAgent() = delete;
    virtual ~TraderAgent() = default;

    TraderAgent(asio::io_context& io_context, int agent_id, unsigned int port)
    : Agent(io_context, agent_id, port)
    {
    }

    TraderAgent(asio::io_context& io_context, int agent_id)
    : Agent(io_context, agent_id)
    {
    }

    /** Subscribes to updates for the stock with the given ticker at the given exchange. */
    void subscribeToMarket(std::string_view exchange, std::string_view ticker);

    /** Places a limit order for the given ticker at the given exchange. */
    void placeLimitOrder(std::string_view exchange, Order::Side side, std::string_view ticker, int quantity, double price);

    // /** Places a market order for the given ticker at the given exchange. */
    void placeMarketOrder(std::string_view exchange, Order::Side side, std::string_view ticker, int quantity);

    // /** Cancels the order with the given id at the given exchange. */
    void cancelOrder(std::string_view exchange, int order_id);
    

    /** Derived classes must implement these: */

    /** The callback function called when trading window starts. */
    virtual void onTradingStart() = 0;

    /** The callback function called when trading window ends. */
    virtual void onTradingEnd() = 0;

    /** The callback function called when new market data update is received. */
    virtual void onMarketData(std::string_view exchange, MarketDataMessagePtr msg) = 0;

    /** The callback function called when the execution report message is received. */
    virtual void onExecutionReport(std::string_view exchange, ExecutionReportMessagePtr msg) = 0;

private:

    /** Checks the type of the incoming message and makes a callback. */
    std::optional<MessagePtr> handleMessageFrom(std::string_view sender, MessagePtr message) override;

    /** Checks the type of the incoming broadcast and makes a callback. */
    void handleBroadcastFrom(std::string_view sender, MessagePtr message) override;
};

#endif