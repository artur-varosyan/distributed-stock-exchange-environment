#ifndef STOCK_EXCHANGE_HPP
#define STOCK_EXCHANGE_HPP

#include "exchangeagent.hpp"
#include "../order/order.hpp"
#include "../order/orderbook.hpp"
#include "../utilities/syncqueue.hpp"

class StockExchange : public ExchangeAgent
{
public:

    StockExchange(asio::io_context& io_context, int agent_id, std::string_view exchange_name, unsigned int port)
    : ExchangeAgent(io_context, agent_id, exchange_name, port),
      order_books_{},
      subscribers_{},
      order_queue_{},
      market_update_queue_{}
    {
    }

    StockExchange(asio::io_context& io_context, int agent_id, std::string_view exchange_name)
    : ExchangeAgent(io_context, agent_id, exchange_name),
      order_books_{},
      subscribers_{},
      order_queue_{},
      market_update_queue_{}
    {
    }

    /** Starts the exchange. */
    void start() override;

    /** Adds the given asset a tradeable and initialises an empty order book. */
    void addTradeableAsset(std::string_view ticker);

    /** Publishes market data to all subscribers. */
    void publishMarketData(std::string_view ticker);

    void onLimitOrder(LimitOrderMessagePtr msg) override;
    void onMarketOrder(MarketOrderMessagePtr msg) override;
    void onCancelOrder(CancelOrderMessagePtr msg) override;
    void onSubscribe(SubscribeMessagePtr msg) override;

private:

    /** Runs the matching engine. */
    void runMatchingEngine();

    /** Runs the market data publisher. */
    void runMarketDataPublisher();

    /** Checks if the given order crosses the spread. */
    bool crossesSpread(OrderPtr order);

    /** Matches the given order with the orders currently present in the OrderBook */
    void matchOrders(OrderPtr order);

    /** Queues a market update for the given ticker. */
    void queueMarketUpdate(std::string_view ticker);

    /** Order books for each ticker traded. */
    std::unordered_map<std::string, OrderBook> order_books_;

    /** Subscribers for each ticker traded. */
    std::unordered_map<std::string, std::vector<std::string>> subscribers_;

    /** Thread-safe FIFO queue for incoming orders. */
    SyncQueue<OrderPtr> order_queue_;

    /** Thread-safe market update queue. */
    SyncQueue<OrderBook::Summary> market_update_queue_;
};

#endif