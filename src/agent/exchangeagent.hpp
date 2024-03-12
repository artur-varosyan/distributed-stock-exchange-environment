#ifndef EXCHANGE_AGENT_HPP
#define EXCHANGE_AGENT_HPP

#include <iostream>
#include <unordered_map>

#include <boost/asio.hpp>

#include "agent.hpp"
#include "../order/order.hpp"
#include "../message/market_order_message.hpp"
#include "../message/limit_order_message.hpp"
#include "../message/cancel_order_message.hpp"
#include "../message/order_ack_message.hpp"
#include "../message/subscribe_message.hpp"
#include "../message/market_data_message.hpp"

namespace asio = boost::asio;

class ExchangeAgent : public Agent
{
public:

    ExchangeAgent() = delete;
    virtual ~ExchangeAgent() = default;

    ExchangeAgent(asio::io_context& io_context, int agent_id, std::string_view exchange_name, unsigned int port)
    : Agent(io_context, agent_id, port),
      exchange_name_{exchange_name}
    {
    }

    ExchangeAgent(asio::io_context& io_context, int agent_id, std::string_view exchange_name)
    : Agent(io_context, agent_id),
      exchange_name_{exchange_name}
    {
    }

    virtual void onLimitOrder(LimitOrderMessagePtr msg) = 0;
    virtual void onMarketOrder(MarketOrderMessagePtr msg) = 0;
    virtual void onCancelOrder(CancelOrderMessagePtr msg) = 0;
    virtual void onSubscribe(SubscribeMessagePtr msg) = 0;

protected:

    /** The unique name of the exchange*/
    std::string exchange_name_;

    /** Checks the type of the incoming message and makes a callback. */
    std::optional<MessagePtr> handleMessageFrom(std::string_view sender, MessagePtr message) override;

    /** Checks the type of the incoming broadcast and makes a callback. */
    void handleBroadcastFrom(std::string_view sender, MessagePtr message) override;
};

#endif