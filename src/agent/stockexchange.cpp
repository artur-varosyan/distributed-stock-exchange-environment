#include "stockexchange.hpp"

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