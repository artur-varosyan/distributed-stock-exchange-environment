#include "exchangeagent.hpp"

void ExchangeAgent::addTradeableAsset(std::string_view ticker)
{
    order_books_.insert({std::string{ticker}, OrderBook(ticker)});
    subscribers_.insert({std::string{ticker}, {}});
}

void ExchangeAgent::publishMarketData(std::string_view ticker)
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
}

std::optional<MessagePtr> ExchangeAgent::handleMessageFrom(std::string_view sender, MessagePtr message)
{
    switch (message->type)
    {
        case MessageType::LIMIT_ORDER:
        {
            LimitOrderMessagePtr msg = std::dynamic_pointer_cast<LimitOrderMessage>(message);
            if (msg == nullptr) {
                throw std::runtime_error("Failed to cast message to LimitOrderMessage");
            }
            onLimitOrder(msg);
            break;
        }
        case MessageType::MARKET_ORDER:
        {
            MarketOrderMessagePtr msg = std::dynamic_pointer_cast<MarketOrderMessage>(message);
            if (msg == nullptr) {
                throw std::runtime_error("Failed to cast message to MarketOrderMessage");
            }
            onMarketOrder(msg);
            break;
        }
        case MessageType::CANCEL_ORDER:
        {
            CancelOrderMessagePtr msg = std::dynamic_pointer_cast<CancelOrderMessage>(message);
            if (msg == nullptr) {
                throw std::runtime_error("Failed to cast message to CancelOrderMessage");
            }
            onCancelOrder(msg);
            break;
        }
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
            throw std::runtime_error("Unsupported message type for exchange");
    }
    return std::nullopt;
}

void ExchangeAgent::handleBroadcastFrom(std::string_view sender, MessagePtr message)
{
    /** TODO: Decide how to handle this more elegantly */
    throw std::runtime_error("ExchangeAgent does not handle broadcasts.");
}


void ExchangeAgent::onSubscribe(SubscribeMessagePtr msg)
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
}

void ExchangeAgent::onLimitOrder(LimitOrderMessagePtr msg)
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
}

void ExchangeAgent::onMarketOrder(MarketOrderMessagePtr msg)
{
    throw std::runtime_error("Implement me!");
}

void ExchangeAgent::onCancelOrder(CancelOrderMessagePtr msg)
{
    throw new std::runtime_error("Implement me!");
}