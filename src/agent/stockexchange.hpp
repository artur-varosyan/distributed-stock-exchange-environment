#ifndef STOCK_EXCHANGE_HPP
#define STOCK_EXCHANGE_HPP

#include "../agent/agent.hpp"
#include "../order/order.hpp"
#include "../order/orderbook.hpp"
#include "../order/trade.hpp"
#include "../order/orderfactory.hpp"
#include "../order/tradefactory.hpp"
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

    StockExchange(asio::io_context& io_context, int agent_id, std::string_view exchange_name, unsigned int port)
    : Agent(io_context, agent_id, port),
      exchange_name_{exchange_name},
      order_books_{},
      subscribers_{},
      msg_queue_{},
      market_update_queue_{},
      trade_tape_{}
    {
    }

    StockExchange(asio::io_context& io_context, int agent_id, std::string_view exchange_name)
    : Agent(io_context, agent_id),
      exchange_name_{exchange_name},
      order_books_{},
      subscribers_{},
      msg_queue_{},
      market_update_queue_{},
      trade_tape_{}
    {
    }

    /** Starts the exchange. */
    void start() override;

    /** Adds the given asset as tradeable and initialises an empty order book. */
    void addTradeableAsset(std::string_view ticker);

    /** Sends execution report to the trader. */
    void sendExecutionReport(std::string_view trader, ExecutionReportMessagePtr msg);

    /** Publishes market data to all subscribers. */
    void publishMarketData(std::string_view ticker);

    /** Starts trading session and informs all market data subscribers. */
    void startTradingSession();

    /** Ends trading session and informs all market data subscribers. */
    void endTradingSession();

    /** Returns the pointer to the order book for the given ticker. */
    OrderBookPtr getOrderBookFor(std::string_view ticker);

    /** Adds the given subscriber to the market data subscribers list. */
    void addSubscriber(std::string_view ticker, int subscriber_id, std::string_view address);

private:

    /** Runs the matching engine. */
    void runMatchingEngine();

    /** Runs the market data publisher. */
    void runMarketDataPublisher();

    /** Checks if the given order crosses the spread. */
    bool crossesSpread(LimitOrderPtr order);

    /** Matches the given order with the orders currently present in the OrderBook */
    void matchOrders(LimitOrderPtr order);

    /** Executes the trade between the resting and aggressing orders. */
    void executeTrade(LimitOrderPtr resting_order, OrderPtr aggressing_order, TradePtr trade);

    /** Adds the given trade to the trade tape. */
    void addTradeToTape(TradePtr trade);

    /** Handles a market order message. 
     * The exchange only accepts Immediate or Cancel (IOC) orders. */
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

    /** The unique name of the exchange*/
    std::string exchange_name_;

    /** Order books for each ticker traded. */
    std::unordered_map<std::string, OrderBookPtr> order_books_;

    /** Subscribers for each ticker traded. */
    std::unordered_map<std::string, std::unordered_map<int, std::string>> subscribers_;

    /** Thread-safe FIFO queue for incoming messages to be processed by the matching engine. */
    SyncQueue<MessagePtr> msg_queue_;

    /** Thread-safe market update queue. */
    SyncQueue<OrderBook::Summary> market_update_queue_;

    OrderFactory order_factory_;
    TradeFactory trade_factory_;

    std::vector<CSVPrintablePtr> trade_tape_;

    /** Conditional variable signalling whether trading window is open */
    bool trading_window_open_ = false;
    std::mutex trading_window_mutex_;
    std::condition_variable trading_window_cv_;
};

#endif