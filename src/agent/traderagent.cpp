#include <iostream>
#include <optional>

#include "traderagent.hpp"

std::optional<MessagePtr> TraderAgent::handleMessageFrom(std::string_view sender, MessagePtr message)
{
    std::cout << "Trader agent handling message" << "\n";
    std::cout << "Type:" << static_cast<int>(message->type) << "\n";
    std::cout << "Sender: " << message->sender_id << "\n";
    std::cout << "Timestamp: " << message->timestamp << "\n";

    switch (message->type)
    {
        case MessageType::MARKET_DATA: 
        {
            MarketDataMessagePtr msg = std::dynamic_pointer_cast<MarketDataMessage>(message);
            if (msg == nullptr) {
                throw std::runtime_error("Failed to cast message to MarketDataMessage");
            }
            onMarketData(sender, msg);
        }
        case MessageType::ORDER_ACK:
        {
            OrderAckMessagePtr msg = std::dynamic_pointer_cast<OrderAckMessage>(message);
            if (msg == nullptr) {
                throw std::runtime_error("Failed to cast message to OrderAckMessage");
            }
            onOrderAck(sender, msg);
        }
        default:
        {
            std::cout << "Unknown message type" << "\n";
            break;
        }
    }
    
    return std::nullopt;
}

void TraderAgent::handleBroadcastFrom(std::string_view sender, MessagePtr message)
{
    std::cout << "Trader agent handling broadcast" << "\n";
    std::cout << "Type:" << static_cast<int>(message->type) << "\n";
    std::cout << "Sender: " << message->sender_id << "\n";
    std::cout << "Timestamp: " << message->timestamp << "\n";

    switch (message->type)
    {
        case MessageType::MARKET_DATA: 
        {
            MarketDataMessagePtr msg = std::dynamic_pointer_cast<MarketDataMessage>(message);
            if (msg == nullptr) {
                throw std::runtime_error("Failed to cast message to MarketDataMessage");
            }
            break;
        }
        case MessageType::ORDER_ACK:
        {
            OrderAckMessagePtr msg = std::dynamic_pointer_cast<OrderAckMessage>(message);
            if (msg == nullptr) {
                throw std::runtime_error("Failed to cast message to OrderAckMessage");
            }
        }
        default:
        {
            std::cout << "Unknown message type" << "\n";
            break;
        }
    }
}

void TraderAgent::subscribeToMarket(std::string_view exchange, std::string_view ticker)
{
    SubscribeMessagePtr msg = std::make_shared<SubscribeMessage>();
    msg->ticker = std::string{ticker};

    Agent::sendMessageTo(exchange, std::dynamic_pointer_cast<Message>(msg));
}

void TraderAgent::placeLimitOrder(std::string_view exchange, std::string_view ticker, int quantity, double price)
{
    LimitOrderMessagePtr msg = std::make_shared<LimitOrderMessage>();
    msg->ticker = std::string{ticker};;
    msg->quantity = quantity;
    msg->price = price;

    Agent::sendMessageTo(exchange, std::dynamic_pointer_cast<Message>(msg));
}

void TraderAgent::placeMarketOrder(std::string_view exchange, std::string_view ticker, int quantity)
{
    MarketOrderMessagePtr msg = std::make_shared<MarketOrderMessage>();
    msg->ticker = std::string{ticker};
    msg->quantity = quantity;

    Agent::sendMessageTo(exchange, std::dynamic_pointer_cast<Message>(msg));
}