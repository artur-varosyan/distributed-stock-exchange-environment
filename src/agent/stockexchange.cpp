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
    OrderPtr order = std::make_shared<Order>(++order_count_);
    order->ticker = msg->ticker;
    order->sender_id = msg->sender_id;
    order->side = msg->side;
    order->price = msg->price;
    order->quantity = msg->quantity;
    std::cout << *order << "\n";

    if (crossesSpread(order))
    {
        // std::cout << "Order crosses spread" << "\n";
        matchOrders(order);
    }
    else
    {
        getOrderBookFor(order->ticker)->addOrder(order);
    }

    publishMarketData(msg->ticker);
};

void StockExchange::onMarketOrder(MarketOrderMessagePtr msg)
{
    throw std::runtime_error("Market orders not yet implemented");
};

void StockExchange::onCancelOrder(CancelOrderMessagePtr msg)
{
    throw std::runtime_error("Cancel orders not yet implemented");
};

bool StockExchange::crossesSpread(OrderPtr order)
{
    if (order->side == Order::Side::BID)
    {
        std::optional<OrderPtr> best_ask = getOrderBookFor(order->ticker)->bestAsk();
        if (best_ask.has_value() && order->price >= best_ask.value()->price)
        {
            return true;
        }
    }
    else
    {
        std::optional<OrderPtr> best_bid = getOrderBookFor(order->ticker)->bestBid();
        if (best_bid.has_value() && order->price <= best_bid.value()->price)
        {
            return true;
        }
    }
    return false;
};

void StockExchange::matchOrders(OrderPtr order)
{
    /** TODO: Ensure that the same trader cannot trade with themselves. */
    if (order->side == Order::Side::BID) {
        std::optional<OrderPtr> best_ask = getOrderBookFor(order->ticker)->bestAsk();

        while (best_ask.has_value() && order->quantity > 0 && order->price >= best_ask.value()->price)
        {
            getOrderBookFor(order->ticker)->popBestAsk();
            double trade_price = best_ask.value()->price;

            // std::cout << "Order executed against: " << *best_ask.value() << "\n";
            std::cout << "Trade price: $" << trade_price << "\n";

            // If the incoming order can be executed in full
            if (order->quantity <= best_ask.value()->quantity) {
                best_ask.value()->quantity -= order->quantity;
                order->quantity = 0;
            }
            // Partially execute incoming order against the best ask
            else {
                order->quantity -= best_ask.value()->quantity;
                best_ask.value()->quantity = 0;
            }

            // Re-insert the best ask if it has not been fully executed
            if (best_ask.value()->quantity > 0) {
                getOrderBookFor(order->ticker)->addOrder(best_ask.value());
            }

            /** TODO: Create a trade, add to log and inform traders */

            best_ask = getOrderBookFor(order->ticker)->bestAsk();
        }
    }
    else
    {
        std::optional<OrderPtr> best_bid = getOrderBookFor(order->ticker)->bestBid();

        while (best_bid.has_value() && order->quantity > 0 && order->price <= best_bid.value()->price)
        {
            getOrderBookFor(order->ticker)->popBestBid();
            double trade_price = best_bid.value()->price;
            // std::cout << "Order executed against: " << *best_bid.value() << "\n";
            std::cout << "Trade price: $" << trade_price << "\n";

            // If the incoming order can be executed in full
            if (order->quantity <= best_bid.value()->quantity) {
                best_bid.value()->quantity -= order->quantity;
                order->quantity = 0;
            }
            // Partially execute incoming order against the best bid
            else {
                order->quantity -= best_bid.value()->quantity;
                best_bid.value()->quantity = 0;
            }

            // Re-insert the best bid if it has not been fully executed
            if (best_bid.value()->quantity > 0) {
                getOrderBookFor(order->ticker)->addOrder(best_bid.value());
            }

            /** TODO: Create a trade, add to log and inform traders */
            best_bid = getOrderBookFor(order->ticker)->bestBid();
        }
    }

    // If the incoming order is not fully executed, add it to the order book
    if (order->quantity > 0) {
        // std::cout << "Adding remaining order to book" << "\n";
        // std::cout << *order << "\n";
        getOrderBookFor(order->ticker)->addOrder(order);
    } else {
        // std::cout << "Order fully executed" << "\n";
    }
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
    // std::cout << summary << "\n";
    
    MarketDataMessagePtr msg = std::make_shared<MarketDataMessage>();
    msg->summary = summary;

    /** Send message to all subscribers of the given ticker */
    /** TODO: This should be randomised or UDP multicast */

    for (auto const& [subscriber_id, address] : subscribers_.at(std::string{ticker}))
    {
        // std::cout << "Sending market data to " << subscriber_id << " @ " << address << "\n";
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