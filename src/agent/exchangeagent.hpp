#ifndef EXCHANGE_AGENT_HPP
#define EXCHANGE_AGENT_HPP

#include <iostream>
#include <unordered_map>

#include <boost/asio.hpp>

#include "agent.hpp"
#include "../order/order.hpp"
#include "../order/orderbook.hpp"
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
      exchange_name_{exchange_name},
      order_books_{},
      subscribers_{}
    {
    }

    ExchangeAgent(asio::io_context& io_context, int agent_id, std::string_view exchange_name)
    : Agent(io_context, agent_id),
      exchange_name_{exchange_name},
      order_books_{},
      subscribers_{}
    {
    }

    /** Adds the given asset a tradeable and initialises an empty order book. */
    void addTradeableAsset(std::string_view ticker);

    /** Publishes market data to all subscribers. */
    void publishMarketData(std::string_view ticker);

    void onLimitOrder(LimitOrderMessagePtr msg);
    void onMarketOrder(MarketOrderMessagePtr msg);
    void onCancelOrder(CancelOrderMessagePtr msg);
    void onSubscribe(SubscribeMessagePtr msg);

private:

    /** Checks the type of the incoming message and makes a callback. */
    std::optional<MessagePtr> handleMessageFrom(std::string_view sender, MessagePtr message) override;

    /** Checks the type of the incoming broadcast and makes a callback. */
    void handleBroadcastFrom(std::string_view sender, MessagePtr message) override;

    /** The unique name of the exchange*/
    std::string exchange_name_;

    /** Order books for each ticker traded. */
    std::unordered_map<std::string, OrderBook> order_books_;

    /** Subscribers for each ticker traded. */
    std::unordered_map<std::string, std::vector<std::string>> subscribers_;
};

#endif