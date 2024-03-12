#include "exchangeagent.hpp"

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
};

void ExchangeAgent::handleBroadcastFrom(std::string_view sender, MessagePtr message)
{
    /** TODO: Decide how to handle this more elegantly. */
    throw std::runtime_error("ExchangeAgent does not handle broadcasts");
};