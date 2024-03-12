#include <iostream>
#include <optional>

#include "traderagent.hpp"

std::optional<MessagePtr> TraderAgent::handleMessageFrom(std::string_view sender, MessagePtr message)
{
    switch (message->type)
    {
        case MessageType::MARKET_DATA: 
        {
            MarketDataMessagePtr msg = std::dynamic_pointer_cast<MarketDataMessage>(message);
            if (msg == nullptr) {
                throw std::runtime_error("Failed to cast message to MarketDataMessage");
            }
            onMarketData(sender, msg);
            break;
        }
        case MessageType::ORDER_ACK:
        {
            OrderAckMessagePtr msg = std::dynamic_pointer_cast<OrderAckMessage>(message);
            if (msg == nullptr) {
                throw std::runtime_error("Failed to cast message to OrderAckMessage");
            }
            onOrderAck(sender, msg);
            break;
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
    msg->sender_id = this->agent_id;
    msg->ticker = std::string{ticker};
    msg->address = "127.0.0.1:" + std::to_string(this->port);

    Agent::sendMessageTo(exchange, std::dynamic_pointer_cast<Message>(msg));
}

void TraderAgent::placeLimitOrder(std::string_view exchange, Order::Side side, std::string_view ticker, int quantity, double price)
{
    LimitOrderMessagePtr msg = std::make_shared<LimitOrderMessage>();
    msg->sender_id = this->agent_id;
    msg->ticker = std::string{ticker};;
    msg->quantity = quantity;
    msg->price = price;
    msg->side = side;

    Agent::sendMessageTo(exchange, std::dynamic_pointer_cast<Message>(msg));
}

void TraderAgent::placeMarketOrder(std::string_view exchange, Order::Side side, std::string_view ticker, int quantity)
{
    MarketOrderMessagePtr msg = std::make_shared<MarketOrderMessage>();
    msg->sender_id = this->agent_id;
    msg->ticker = std::string{ticker};
    msg->quantity = quantity;
    msg->side = side;

    Agent::sendMessageTo(exchange, std::dynamic_pointer_cast<Message>(msg));
}