#include "stockexchange.hpp"
#include "../utilities/syncqueue.hpp"

void StockExchange::start()
{
    // Create a Matching Engine Thread
    std::thread matching_engine_thread(&StockExchange::runMatchingEngine, this);
    
    // Main thread continues to handle incoming and outgoing communication
    Agent::start();
};

void StockExchange::runMatchingEngine()
{
    // Wait until trading window opens
    std::unique_lock<std::mutex> trading_window_lock(trading_window_mutex_);
    trading_window_cv_.wait(trading_window_lock, [this]{ return trading_window_open_;});

    // Atomically check if trading window is open, and if not break loop
    while (trading_window_open_)
    {
        trading_window_lock.unlock();
        
        // Wait until new message is present
        MessagePtr msg = msg_queue_.pop();

        // Pattern match the message type
        switch (msg->type) {
            case MessageType::MARKET_ORDER:
            {
                onMarketOrder(std::dynamic_pointer_cast<MarketOrderMessage>(msg));
                break;
            }
            case MessageType::LIMIT_ORDER:
            {
                onLimitOrder(std::dynamic_pointer_cast<LimitOrderMessage>(msg));
                break;
            }
            case MessageType::CANCEL_ORDER:
            {
                onCancelOrder(std::dynamic_pointer_cast<CancelOrderMessage>(msg));
                break;
            }
            default:
            {
                std::cout << "Exchange received unknown message type" << "\n";
            }
        }
        trading_window_lock.lock();
    }
    trading_window_lock.unlock();
    std::cout << "Stopped running matching engine" << "\n";
};

void StockExchange::onLimitOrder(LimitOrderMessagePtr msg)
{
    LimitOrderPtr order = order_factory_.createLimitOrder(msg);

    if (crossesSpread(order))
    {
        // std::cout << "Order crosses spread" << "\n";
        matchOrders(order);
    }
    else
    {
        getOrderBookFor(order->ticker)->addOrder(order);
        ExecutionReportMessagePtr report = ExecutionReportMessage::createFromOrder(order, Order::Status::NEW);
        sendExecutionReport(std::to_string(order->sender_id), report);
        publishMarketData(msg->ticker);
    }
};

void StockExchange::onMarketOrder(MarketOrderMessagePtr msg)
{
    MarketOrderPtr order = order_factory_.createMarketOrder(msg);

    if (msg->side == Order::Side::BID)
    {
        std::optional<LimitOrderPtr> best_ask = getOrderBookFor(msg->ticker)->bestAsk();

        while (best_ask.has_value() && !order->isFilled())
        {
            getOrderBookFor(msg->ticker)->popBestAsk();

            TradePtr trade = trade_factory_.createFromLimitAndMarketOrders(best_ask.value(), order);
            addTradeToTape(trade);
            executeTrade(best_ask.value(), order, trade);

            best_ask = getOrderBookFor(order->ticker)->bestAsk();
        }
    }
    else
    {
        std::optional<LimitOrderPtr> best_bid = getOrderBookFor(msg->ticker)->bestBid();

        while (best_bid.has_value() && !order->isFilled())
        {
            getOrderBookFor(msg->ticker)->popBestBid();

            TradePtr trade = trade_factory_.createFromLimitAndMarketOrders(best_bid.value(), order);
            addTradeToTape(trade);
            executeTrade(best_bid.value(), order, trade);

            best_bid = getOrderBookFor(order->ticker)->bestBid();
        }
    }

    // Immediate or Cancel (IOC)
    // If the market order is not fully executed, cancel the remaining quantity
    if (!order->isFilled())
    {
        ExecutionReportMessagePtr report = ExecutionReportMessage::createFromOrder(order, Order::Status::CANCELLED);
        sendExecutionReport(std::to_string(order->sender_id), report);
    }
};

void StockExchange::onCancelOrder(CancelOrderMessagePtr msg)
{
    throw std::runtime_error("Cancel orders not yet implemented");
};

bool StockExchange::crossesSpread(LimitOrderPtr order)
{
    if (order->side == Order::Side::BID)
    {
        std::optional<LimitOrderPtr> best_ask = getOrderBookFor(order->ticker)->bestAsk();
        if (best_ask.has_value() && order->price >= best_ask.value()->price)
        {
            return true;
        }
    }
    else
    {
        std::optional<LimitOrderPtr> best_bid = getOrderBookFor(order->ticker)->bestBid();
        if (best_bid.has_value() && order->price <= best_bid.value()->price)
        {
            return true;
        }
    }
    return false;
};

void StockExchange::matchOrders(LimitOrderPtr order)
{
    if (order->side == Order::Side::BID) {
        std::optional<LimitOrderPtr> best_ask = getOrderBookFor(order->ticker)->bestAsk();
        
        while (best_ask.has_value() && !order->isFilled() && order->price >= best_ask.value()->price)
        {
            getOrderBookFor(order->ticker)->popBestAsk();

            TradePtr trade = trade_factory_.createFromLimitOrders(best_ask.value(), order);
            addTradeToTape(trade);
            executeTrade(best_ask.value(), order, trade);

            best_ask = getOrderBookFor(order->ticker)->bestAsk();
        }
    }
    else
    {
        std::optional<LimitOrderPtr> best_bid = getOrderBookFor(order->ticker)->bestBid();

        while (best_bid.has_value() && !order->isFilled() && order->price <= best_bid.value()->price)
        {
            getOrderBookFor(order->ticker)->popBestBid();

            TradePtr trade = trade_factory_.createFromLimitOrders(best_bid.value(), order);
            addTradeToTape(trade);
            executeTrade(best_bid.value(), order, trade);

            best_bid = getOrderBookFor(order->ticker)->bestBid();
        }
    }

    // If the incoming order is not fully executed, add it to the order book
    if (!order->isFilled()){
        getOrderBookFor(order->ticker)->addOrder(order);
    }
};

void StockExchange::executeTrade(LimitOrderPtr resting_order, OrderPtr aggressing_order, TradePtr trade)
{
    // Decrement the quantity of the orders by quantity traded
    resting_order->updateOrderWithTrade(trade);
    aggressing_order->updateOrderWithTrade(trade);

    // Re-insert the resting order if it has not been fully filled
    if (resting_order->remaining_quantity > 0) {
        getOrderBookFor(resting_order->ticker)->addOrder(resting_order);
    }

    // Log the trade in the order book
    getOrderBookFor(resting_order->ticker)->logTrade(trade);

    // Send execution reports to the traders
    ExecutionReportMessagePtr resting_report = ExecutionReportMessage::createFromTrade(resting_order, trade);
    ExecutionReportMessagePtr aggressing_report = ExecutionReportMessage::createFromTrade(aggressing_order, trade);
    sendExecutionReport(std::to_string(resting_order->sender_id), resting_report);
    sendExecutionReport(std::to_string(aggressing_order->sender_id), aggressing_report);

    // Broadcast the market data to all subscribers
    publishMarketData(resting_order->ticker);
}

void StockExchange::sendExecutionReport(std::string_view trader, ExecutionReportMessagePtr msg)
{
    sendMessageTo(trader, std::dynamic_pointer_cast<Message>(msg));
};

std::optional<MessagePtr> StockExchange::handleMessageFrom(std::string_view sender, MessagePtr message)
{
    switch (message->type)
    {
        case MessageType::SUBSCRIBE:
        {
            SubscribeMessagePtr msg = std::dynamic_pointer_cast<SubscribeMessage>(message);
            if (msg == nullptr) {
                throw std::runtime_error("Failed to cast message to SubscribeMessage");
            }
            onSubscribe(msg);
            break;
        }
        default:
        {
            // Send message to the matching engine
            msg_queue_.push(message);
        }
    }
    return std::nullopt;
};

void StockExchange::handleBroadcastFrom(std::string_view sender, MessagePtr message)
{
    /** TODO: Decide how to handle this more elegantly. */
    throw std::runtime_error("ExchangeAgent does not handle broadcasts");
};


void StockExchange::onSubscribe(SubscribeMessagePtr msg)
{
    if (order_books_.contains(std::string{msg->ticker}))
    {
        std::cout << "Subscription address: " << msg->address << "\n";
        addSubscriber(msg->ticker, msg->sender_id, msg->address);
    }
    else
    {
        throw std::runtime_error("Failed to add subscriber: Ticker " + msg->ticker + " not found");
    }
};

void StockExchange::addSubscriber(std::string_view ticker, int subscriber_id, std::string_view address)
{
    subscribers_.at(std::string{ticker}).insert({subscriber_id, std::string{address}});
};

void StockExchange::addTradeableAsset(std::string_view ticker)
{
    order_books_.insert({std::string{ticker}, OrderBook::create(ticker)});
    subscribers_.insert({std::string{ticker}, {}});
};

void StockExchange::publishMarketData(std::string_view ticker)
{
    OrderBook::Summary summary = getOrderBookFor(ticker)->getSummary();
    
    MarketDataMessagePtr msg = std::make_shared<MarketDataMessage>();
    msg->summary = summary;

    /** Send message to all subscribers of the given ticker */
    /** TODO: This should be randomised or UDP multicast */

    for (auto const& [subscriber_id, address] : subscribers_.at(std::string{ticker}))
    {
        sendBroadcast(address, std::dynamic_pointer_cast<Message>(msg));
    }
};

void StockExchange::startTradingSession()
{
    // Signal start of trading window to the matching engine
    std::unique_lock<std::mutex> trading_window_lock(trading_window_mutex_);
    trading_window_open_ = true;
    trading_window_lock.unlock();
    trading_window_cv_.notify_all();

    EventMessagePtr msg = std::make_shared<EventMessage>(EventMessage::EventType::TRADING_SESSION_START); 

    // Send a message to subscribers of all tickers
    for (auto const& [ticker, ticker_subscribers] : subscribers_)
    {
        for (auto const& [subscriber_id, address] : ticker_subscribers)
        {
            sendBroadcast(address, std::dynamic_pointer_cast<Message>(msg));
        }
    }
};

void StockExchange::endTradingSession()
{
    // Signal end of trading window to the matching engine
    std::unique_lock<std::mutex> trading_window_lock(trading_window_mutex_);
    trading_window_open_ = false;
    trading_window_lock.unlock();
    trading_window_cv_.notify_all();

    EventMessagePtr msg = std::make_shared<EventMessage>(EventMessage::EventType::TRADING_SESSION_END);

    // Send a message to subscribers of all tickers
    for (auto const& [ticker, ticker_subscribers] : subscribers_)
    {
        for (auto const& [subscriber_id, address] : ticker_subscribers)
        {
            sendBroadcast(address, std::dynamic_pointer_cast<Message>(msg));
        }
    }
};

OrderBookPtr StockExchange::getOrderBookFor(std::string_view ticker)
{
    return order_books_.at(std::string{ticker});
};

void StockExchange::addTradeToTape(TradePtr trade)
{
    /** TODO: Log trades to a CSV file */
    // std::cout << *trade << "\n";
    trade_tape_.push_back(trade);
};