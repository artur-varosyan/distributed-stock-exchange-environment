#include "stockexchange.hpp"
#include "../utilities/syncqueue.hpp"

void StockExchange::start()
{
    // 1. Create a Matching Engine Thread
    std::thread matching_engine_thread(&StockExchange::runMatchingEngine, this);
    // 2. Create a Market Data Publisher Thread
    std::thread data_publisher_thread(&StockExchange::runMarketDataPublisher, this);
    
    // 3. Main thread continues to listen for incoming messages
    Agent::start();
};

void StockExchange::runMatchingEngine()
{
    std::cout << "Started running matching engine" << "\n";

    /** TODO: Add stopping condition */
    while (true)
    {
        // Wait until new order is present
        OrderPtr order = order_queue_.pop();
        
        /** TODO: Match orders */
        // matchOrders(order);

        /** TODO: Add market update to queue */
    }
    std::cout << "Stopped running matching engine" << "\n";
};

bool StockExchange::crossesSpread(OrderPtr order)
{
    throw new std::runtime_error("Implement me!");
};

void StockExchange::matchOrders(OrderPtr order)
{
    throw new std::runtime_error("Implement me!");
};

void StockExchange::queueMarketUpdate(std::string_view ticker)
{
    throw new std::runtime_error("Implement me!");
};

void StockExchange::runMarketDataPublisher()
{
    std::cout << "Started running market data publisher" << "\n";

    /** TODO: Add stopping condition */
    while (true)
    {
        // Wait until 
        OrderBook::Summary summary = market_update_queue_.pop();
        
        /** TODO: Broadcast the update */
    }
    std::cout << "Stopped running market data publisher" << "\n";
};

void StockExchange::onSubscribe(SubscribeMessagePtr msg)
{
    if (order_books_.find(msg->ticker) != order_books_.end())
    {
        std::string subscriber_id = std::to_string(msg->sender_id);
        std::cout << "Subscription address: " << msg->address << "\n";
        addToAddressBook(msg->address, subscriber_id);
        subscribers_.at(msg->ticker).push_back(subscriber_id);
    }
    else
    {
        throw std::runtime_error("Failed to add subscriber: Ticker " + msg->ticker + " not found");
    }
};

void StockExchange::onLimitOrder(LimitOrderMessagePtr msg)
{
    // Temporary implementation for testing

    OrderPtr order = std::make_shared<Order>(0);
    order->ticker = msg->ticker;
    order->side = msg->side;
    order->quantity = msg->quantity;
    order->price = msg->price;
    order->sender_id = msg->sender_id;

    std::cout << "New limit order msg received" << "\n";
    std::cout << *order << "\n";

    order_books_.at(msg->ticker).addOrder(order);
    publishMarketData(msg->ticker);
};

void StockExchange::onMarketOrder(MarketOrderMessagePtr msg)
{
    throw std::runtime_error("Implement me!");
};

void StockExchange::onCancelOrder(CancelOrderMessagePtr msg)
{
    throw new std::runtime_error("Implement me!");
};

void StockExchange::addTradeableAsset(std::string_view ticker)
{
    order_books_.insert({std::string{ticker}, OrderBook(ticker)});
    subscribers_.insert({std::string{ticker}, {}});
}

void StockExchange::publishMarketData(std::string_view ticker)
{
    OrderBook::Summary summary = order_books_.at(std::string{ticker}).getSummary();
    std::cout << summary << "\n";
    
    MarketDataMessagePtr msg = std::make_shared<MarketDataMessage>();
    msg->summary = summary;

    /** Send message to all subscribers of the given ticker */
    /** TODO: This should be randomised or UDP multicast */
    for (std::string_view subscriber_id : subscribers_.at(std::string{ticker}))
    {
        std::cout << "Sending market data to subscriber: " << subscriber_id << "\n";
        sendBroadcastTo(subscriber_id, msg);
    };
};