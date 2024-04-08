#ifndef STOCK_EXCHANGE_HPP
#define STOCK_EXCHANGE_HPP

#include <random>

#include "../agent/agent.hpp"
#include "../config/exchangeconfig.hpp"
#include "../order/order.hpp"
#include "../order/orderbook.hpp"
#include "../order/orderfactory.hpp"
#include "../trade/trade.hpp"
#include "../trade/tradefactory.hpp"
#include "../utilities/syncqueue.hpp"
#include "../utilities/csvwriter.hpp"
#include "../utilities/csvprintable.hpp"
#include "../message/message.hpp"
#include "../message/market_data_message.hpp"
#include "../message/limit_order_message.hpp"
#include "../message/market_order_message.hpp"
#include "../message/cancel_order_message.hpp"
#include "../message/subscribe_message.hpp"
#include "../message/exec_report_message.hpp"
#include "../message/event_message.hpp"
#include "../message/cancel_reject_message.hpp"

class StockExchange : public Agent
{
public:

    StockExchange(NetworkEntity *network_entity, ExchangeConfigPtr config)
    : Agent(network_entity, std::static_pointer_cast<AgentConfig>(config)),
      exchange_name_{config->name},
      order_books_{},
      subscribers_{},
      trade_tapes_{},
      msg_queue_{},
      random_generator_{std::random_device{}()}
    {
      // Add all tickers to exchange
      for (auto ticker : config->tickers)
      {
        addTradeableAsset(ticker);
      }

      // Set trading window
      setTradingWindow(config->connect_time, config->trading_time);
    }

    /** Starts the exchange. */
    void start() override;

    /** Adds the given asset as tradeable and initialises an empty order book. */
    void addTradeableAsset(std::string_view ticker);

    /** Waits for incoming connections then opens trading window for the specified duration (seconds). */
    void setTradingWindow(int &connect_time, int &trading_time);

    /** Starts trading session and informs all market data subscribers. */
    void startTradingSession();

    /** Ends trading session and informs all market data subscribers. */
    void endTradingSession();

    /** Returns the pointer to the order book for the given ticker. */
    OrderBookPtr getOrderBookFor(std::string_view ticker);

    /** Returns the trade tape writer for the given ticker. */
    CSVWriterPtr getTradeTapeFor(std::string_view ticker);

    /** Adds the given subscriber to the market data subscribers list. */
    void addSubscriber(std::string_view ticker, int subscriber_id, std::string_view address);

private:

    /**
     *   HELPER METHODS
    */

    /** Runs the matching engine. */
    void runMatchingEngine();

    /** Checks if the given order crosses the spread. */
    bool crossesSpread(LimitOrderPtr order);

    /** Matches the given order with the orders currently present in the OrderBook. */
    void matchWithOrderBook(LimitOrderPtr order);

    /** Executes the trade between the resting and aggressing orders. */
    void executeTrade(LimitOrderPtr resting_order, OrderPtr aggressing_order, TradePtr trade);

    /** Adds the given trade to the trade tape. */
    void addTradeToTape(TradePtr trade);

    /** Creates a new trade tape CSV file. */
    void createTradeTape(std::string_view ticker);

    /**
     *   MESSAGE SENDERS
    */

    /** Sends execution report to the trader. */
    void sendExecutionReport(std::string_view trader, ExecutionReportMessagePtr msg);

    /** Publishes market data to all subscribers. */
    void publishMarketData(std::string_view ticker);

    /** Broadcasts the given message to all subscribers of the given ticker. */
    void broadcastToSubscribers(std::string_view ticker, MessagePtr msg);

    /**
     *   MESSAGE HANDLERS
    */

    /** Handles a market order message. Immediate or Cancel (IOC) orders only. */
    void onMarketOrder(MarketOrderMessagePtr msg);

    /** Handles a limit order message. */
    void onLimitOrder(LimitOrderMessagePtr msg);

    /** Handles a cancel order message. */
    void onCancelOrder(CancelOrderMessagePtr msg);

    /** Handles a subscription to market data request message. */
    void onSubscribe(SubscribeMessagePtr msg);

    /** Checks the type of the incoming message and makes a callback. */
    std::optional<MessagePtr> handleMessageFrom(std::string_view sender, MessagePtr message) override;

    /** Checks the type of the incoming broadcast and makes a callback. */
    void handleBroadcastFrom(std::string_view sender, MessagePtr message) override;

    /**
     *   PRIVATE MEMBERS
    */

    /** The unique name of the exchange*/
    std::string exchange_name_;

    /** Order books for each ticker traded. */
    std::unordered_map<std::string, OrderBookPtr> order_books_;

    /** Trade tape for each ticker traded. */
    std::unordered_map<std::string, CSVWriterPtr> trade_tapes_;

    /** Subscribers for each ticker traded. */
    std::unordered_map<std::string, std::unordered_map<int, std::string>> subscribers_;

    /** Thread-safe FIFO queue for incoming messages to be processed by the matching engine. */
    SyncQueue<MessagePtr> msg_queue_;

    OrderFactory order_factory_;
    TradeFactory trade_factory_;

    /** Conditional variable signalling whether trading window is open */
    bool trading_window_open_ = false;
    std::mutex trading_window_mutex_;
    std::condition_variable trading_window_cv_;
    std::thread* trading_window_thread_ = nullptr;
    std::thread* matching_engine_thread_ = nullptr;

    /** Used for randomising the order of UDP broadcasts */
    std::mt19937 random_generator_;
};

#endif