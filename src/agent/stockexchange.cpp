#include <algorithm>
#include <stack>

#include "stockexchange.hpp"
#include "../utilities/syncqueue.hpp"

void StockExchange::start()
{
    // Create a Matching Engine Thread
    matching_engine_thread_ = new std::thread(&StockExchange::runMatchingEngine, this);
    
    // Main thread continues to handle incoming and outgoing communication
    Agent::start();
};

void StockExchange::terminate()
{
    // Only safe if called after matching engine and trading window threads terminate
    matching_engine_thread_->join();
    trading_window_thread_->join();
    delete(matching_engine_thread_);
    delete(trading_window_thread_);
}

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

        msg->markProcessed();
        addMessageToTape(msg);
        
        trading_window_lock.lock();
    }
    
    // Clear the message queue
    std::cout << "Matching Engine stopping. Queue size discarded: " << msg_queue_.size() << "\n";
    msg_queue_.close();

    trading_window_lock.unlock();
    std::cout << "Stopped running matching engine" << "\n";
    // trading_window_cv_.notify_all();
};

void StockExchange::onLimitOrder(LimitOrderMessagePtr msg)
{
    LimitOrderPtr order = order_factory_.createLimitOrder(msg);

    if (crossesSpread(order))
    {
        if (order->time_in_force == Order::TimeInForce::FOK)
        {
            matchOrderInFull(order);
        }
        else
        {
            matchOrder(order);
        }
    }
    else
    {
        getOrderBookFor(order->ticker)->addOrder(order);
        ExecutionReportMessagePtr report = ExecutionReportMessage::createFromOrder(order);
        report->sender_id = this->agent_id;
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

    // If the market order is not fully executed, cancel the remaining quantity
    if (!order->isFilled())
    {
        cancelOrder(order);
    }
};

void StockExchange::onCancelOrder(CancelOrderMessagePtr msg)
{
    std::optional<LimitOrderPtr> order = getOrderBookFor(msg->ticker)->removeOrder(msg->order_id, msg->side);
    
    if (order.has_value()) 
    {
        cancelOrder(order.value());
    }
    else
    {
        // Send a cancel reject message if order does not exist in the order book
        CancelRejectMessagePtr reject = std::make_shared<CancelRejectMessage>();
        reject->sender_id = this->agent_id;
        reject->order_id = msg->order_id;

        sendMessageTo(std::to_string(msg->sender_id), std::dynamic_pointer_cast<Message>(reject), true);
    }
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

void StockExchange::matchOrder(LimitOrderPtr order)
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

    // If the incoming order is Good-Til-Cancelled (GTC) and not fully executed, add it to the order book
    if (!order->isFilled() && order->time_in_force == Order::TimeInForce::GTC){
        getOrderBookFor(order->ticker)->addOrder(order);
    }
    // Cancel the remainder of the order otherwise
    else if (order->time_in_force == Order::TimeInForce::IOC)
    {
        cancelOrder(order);
    }
};

void StockExchange::matchOrderInFull(LimitOrderPtr order)
{
    int temp_rem_quantity = order->remaining_quantity;
    std::stack<LimitOrderPtr> stack;

    if (order->side == Order::Side::BID) {
        std::optional<LimitOrderPtr> best_ask = getOrderBookFor(order->ticker)->bestAsk();
        while (best_ask.has_value() && temp_rem_quantity > 0 && order->price >= best_ask.value()->price)
        {
            getOrderBookFor(order->ticker)->popBestAsk();

            stack.push(best_ask.value());
            temp_rem_quantity -= std::min(temp_rem_quantity, best_ask.value()->remaining_quantity);

            best_ask = getOrderBookFor(order->ticker)->bestAsk();
        }
    }
    else
    {
        std::optional<LimitOrderPtr> best_bid = getOrderBookFor(order->ticker)->bestBid();
        while (best_bid.has_value() && temp_rem_quantity > 0 && order->price <= best_bid.value()->price)
        {
            getOrderBookFor(order->ticker)->popBestBid();

            stack.push(best_bid.value());
            temp_rem_quantity -= std::min(temp_rem_quantity, best_bid.value()->remaining_quantity);

            best_bid = getOrderBookFor(order->ticker)->bestBid();
        }
    }

    // Cancel the order if not executed in full and add all matching orders back to the order book
    if (temp_rem_quantity > 0)
    {
        // Add all matching orders back to the order book
        while (!stack.empty())
        {
            getOrderBookFor(order->ticker)->addOrder(stack.top());
            stack.pop();
        }
        
        // Cancel the incoming order
        cancelOrder(order);
    }
    // Execute the order in full
    else
    {
        while (!stack.empty())
        {
            LimitOrderPtr matched_order = stack.top();
            stack.pop();

            // Execute trade
            TradePtr trade = trade_factory_.createFromLimitOrders(matched_order, order);
            addTradeToTape(trade);
            executeTrade(matched_order, order, trade);
        }
    }
};

void StockExchange::cancelOrder(OrderPtr order)
{
    order->setStatus(Order::Status::CANCELLED);
    ExecutionReportMessagePtr report = ExecutionReportMessage::createFromOrder(order);
    report->sender_id = this->agent_id;
    sendExecutionReport(std::to_string(order->sender_id), report);
}

void StockExchange::executeTrade(LimitOrderPtr resting_order, OrderPtr aggressing_order, TradePtr trade)
{
    // Decrement the quantity of the orders by quantity traded
    getOrderBookFor(resting_order->ticker)->updateOrderWithTrade(resting_order, trade);
    getOrderBookFor(resting_order->ticker)->updateOrderWithTrade(aggressing_order, trade);

    // Re-insert the resting order if it has not been fully filled
    if (resting_order->remaining_quantity > 0) {
        getOrderBookFor(resting_order->ticker)->addOrder(resting_order);
    }

    // Log the trade in the order book
    getOrderBookFor(resting_order->ticker)->logTrade(trade);

    // Send execution reports to the traders
    ExecutionReportMessagePtr resting_report = ExecutionReportMessage::createFromTrade(resting_order, trade);
    resting_report->sender_id = this->agent_id;
    ExecutionReportMessagePtr aggressing_report = ExecutionReportMessage::createFromTrade(aggressing_order, trade);
    aggressing_report->sender_id = this->agent_id;
    sendExecutionReport(std::to_string(resting_order->sender_id), resting_report);
    sendExecutionReport(std::to_string(aggressing_order->sender_id), aggressing_report);

    // Broadcast the market data to all subscribers
    publishMarketData(resting_order->ticker);
}

void StockExchange::sendExecutionReport(std::string_view trader, ExecutionReportMessagePtr msg)
{
    sendMessageTo(trader, std::dynamic_pointer_cast<Message>(msg), true);
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
        std::cout << "Subscription address: " << msg->address << " Agent ID: " << msg->sender_id << "\n";
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

    // If trader connects after trading has started, inform the trader that trading window is open
    std::unique_lock lock {trading_window_mutex_};
    if (trading_window_open_) 
    {
        lock.unlock();

        EventMessagePtr msg = std::make_shared<EventMessage>(EventMessage::EventType::TRADING_SESSION_START); 
        sendBroadcast(address, std::dynamic_pointer_cast<Message>(msg));
    }
    else 
    {
        lock.unlock();
    }
};

void StockExchange::addTradeableAsset(std::string_view ticker)
{
    order_books_.insert({std::string{ticker}, OrderBook::create(ticker)});
    subscribers_.insert({std::string{ticker}, {}});

    createDataFiles(ticker);
    std::cout << "Added " << ticker << " as a tradeable asset" << std::endl;
};

void StockExchange::createDataFiles(std::string_view ticker)
{
    // Get current ISO 8601 timestamp
    std::time_t t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::stringstream ss;
    ss << std::put_time( std::localtime( &t ), "%FT%T" );
    std::string timestamp = ss.str(); 

    // Set path to CSV files
    std::string suffix = std::string{exchange_name_} + "_" + std::string{ticker} + "_"  + timestamp;
    std::string trades_file = "trades_" + suffix + ".csv";
    std::string market_data_file = "data_" + suffix + ".csv";

    // Create a CSV writers
    CSVWriterPtr trade_writer = std::make_shared<CSVWriter>(trades_file);
    CSVWriterPtr market_data_writer = std::make_shared<CSVWriter>(market_data_file);

    trade_tapes_.insert({std::string{ticker}, trade_writer});
    market_data_feeds_.insert({std::string{ticker}, market_data_writer});
}

void StockExchange::createMessageTape() 
{
    // Get current ISO 8601 timestamp
    std::time_t t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::stringstream ss;
    ss << std::put_time( std::localtime( &t ), "%FT%T" );
    std::string timestamp = ss.str(); 

    // Define CSV filename
    std::string suffix = std::string{exchange_name_} + "_"  + timestamp;
    std::string messages_file = "msgs_" + suffix + ".csv";

    // Create message writer
    this->message_tape_ = std::make_shared<CSVWriter>(messages_file);
}

void StockExchange::publishMarketData(std::string_view ticker)
{
    MarketDataPtr data = getOrderBookFor(ticker)->getLiveMarketData();
    addMarketDataSnapshot(data);
    
    MarketDataMessagePtr msg = std::make_shared<MarketDataMessage>();
    msg->data = data;

    // Send message to all subscribers of the given ticker 
    broadcastToSubscribers(ticker, std::dynamic_pointer_cast<Message>(msg));
};

void StockExchange::setTradingWindow(int connect_time, int trading_time)
{
    if (trading_window_thread_ != nullptr)
    {
        throw new std::runtime_error("Trading window already has been set");
    }

    trading_window_thread_ = new std::thread([=, this](){

        // Allow time for connections
        std::cout << "Trading time set to " << trading_time << " seconds." << std::endl;
        std::cout << "Waiting for connections for " << connect_time << " seconds..." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(connect_time));

        // Start trading session 
        std::cout << "Trading session starts now" << std::endl;
        startTradingSession();
        std::this_thread::sleep_for(std::chrono::seconds(trading_time));

        // End trading session
        endTradingSession();
        std::cout << "Trading session ended.\n";
    });
}

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
        broadcastToSubscribers(ticker, std::dynamic_pointer_cast<Message>(msg));
    }
};

void StockExchange::endTradingSession()
{
    // Signal end of trading window to the matching engine
    std::unique_lock<std::mutex> trading_window_lock(trading_window_mutex_);
    trading_window_open_ = false;
    trading_window_lock.unlock();
    trading_window_cv_.notify_all();

    // Iterate through all tickers and close all open csv files
    for (auto const& [ticker, writer] : trade_tapes_)
    {
        writer->stop();
    }

    EventMessagePtr msg = std::make_shared<EventMessage>(EventMessage::EventType::TRADING_SESSION_END);

    // Send a message to subscribers of all tickers
    for (auto const& [ticker, ticker_subscribers] : subscribers_)
    {
        broadcastToSubscribers(ticker, std::dynamic_pointer_cast<Message>(msg));
    }
};

OrderBookPtr StockExchange::getOrderBookFor(std::string_view ticker)
{
    return order_books_.at(std::string{ticker});
};

CSVWriterPtr StockExchange::getTradeTapeFor(std::string_view ticker)
{
    return trade_tapes_.at(std::string{ticker});
};

CSVWriterPtr StockExchange::getMarketDataFeedFor(std::string_view ticker)
{
    return market_data_feeds_.at(std::string{ticker});
};

void StockExchange::addTradeToTape(TradePtr trade)
{
    std::cout << *trade << "\n";
    getTradeTapeFor(trade->ticker)->writeRow(trade);
};

void StockExchange::addMarketDataSnapshot(MarketDataPtr data)
{
    getMarketDataFeedFor(data->ticker)->writeRow(data);
}

void StockExchange::addMessageToTape(MessagePtr msg)
{
    message_tape_->writeRow(msg);
}

void StockExchange::broadcastToSubscribers(std::string_view ticker, MessagePtr msg)
{
    // Randomise the subscribers list
    std::unordered_map<int, std::string> ticker_subcribers(subscribers_.at(std::string{ticker}));
    std::vector<std::pair<int, std::string>> randomised_subscribers(ticker_subcribers.begin(), ticker_subcribers.end());
    std::shuffle(randomised_subscribers.begin(), randomised_subscribers.end(), random_generator_);

    // Send a broadcast to each one
    for (auto const& [subscriber_id, address] : randomised_subscribers)
    {
        sendBroadcast(address, std::dynamic_pointer_cast<Message>(msg));
    }
}