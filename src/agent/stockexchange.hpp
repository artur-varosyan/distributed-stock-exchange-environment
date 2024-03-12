#ifndef STOCK_EXCHANGE_HPP
#define STOCK_EXCHANGE_HPP

#include "exchangeagent.hpp"

class StockExchange : public ExchangeAgent
{
public:

    StockExchange(asio::io_context& io_context, int agent_id, std::string_view exchange_name, unsigned int port)
    : ExchangeAgent(io_context, agent_id, exchange_name, port),
      order_books_{},
      subscribers_{}
    {
    }

    StockExchange(asio::io_context& io_context, int agent_id, std::string_view exchange_name)
    : ExchangeAgent(io_context, agent_id, exchange_name),
      order_books_{},
      subscribers_{}
    {
    }

    /** Adds the given asset a tradeable and initialises an empty order book. */
    void addTradeableAsset(std::string_view ticker);

    /** Publishes market data to all subscribers. */
    void publishMarketData(std::string_view ticker);

    void onLimitOrder(LimitOrderMessagePtr msg) override;
    void onMarketOrder(MarketOrderMessagePtr msg) override;
    void onCancelOrder(CancelOrderMessagePtr msg) override;
    void onSubscribe(SubscribeMessagePtr msg) override;

private:

    /** Order books for each ticker traded. */
    std::unordered_map<std::string, OrderBook> order_books_;

    /** Subscribers for each ticker traded. */
    std::unordered_map<std::string, std::vector<std::string>> subscribers_;
};

#endif