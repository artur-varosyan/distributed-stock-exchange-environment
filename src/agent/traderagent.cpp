#include <iostream>
#include <optional>

#include "traderagent.hpp"

std::optional<MessagePtr> TraderAgent::handleMessageFrom(std::string_view sender, MessagePtr message)
{
    // If trading window (for this trader) not yet open ignore message
    std::unique_lock lock{mutex_};
    if (!trading_window_open_) return std::nullopt;
    lock.unlock();

    switch (message->type)
    {
        case MessageType::EXECUTION_REPORT:
        {
            ExecutionReportMessagePtr msg = std::dynamic_pointer_cast<ExecutionReportMessage>(message);
            if (msg == nullptr) {
                throw std::runtime_error("Failed to cast message to ExecutionReportMessage");
            }
            onExecutionReport(sender, msg);
            break;
        }
        case MessageType::CANCEL_REJECT:
        {
            CancelRejectMessagePtr msg = std::dynamic_pointer_cast<CancelRejectMessage>(message);
            if (msg == nullptr) {
                throw std::runtime_error("Failed to cast message to CancelRejectMessage");
            }
            onCancelReject(sender, msg);
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
            // If trading window (for this trader) not yet open ignore message
            std::unique_lock lock{mutex_};
            if (!trading_window_open_) return;
            lock.unlock();

            MarketDataMessagePtr msg = std::dynamic_pointer_cast<MarketDataMessage>(message);
            if (msg == nullptr) {
                throw std::runtime_error("Failed to cast message to MarketDataMessage");
            }
            onMarketData(sender, msg);
            break;
        }
        case MessageType::EVENT:
        {
            EventMessagePtr msg = std::dynamic_pointer_cast<EventMessage>(message);
            if (msg == nullptr) {
                throw std::runtime_error("Failed to cast message to EventMessage");
            }

            if (msg->event_type == EventMessage::EventType::TRADING_SESSION_START)
            {
                signalTradingStart();
            }
            else if (msg->event_type == EventMessage::EventType::TRADING_SESSION_END)
            {
                onTradingEnd();
            }
            break;
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
    msg->address = "127.0.0.1:" + std::to_string(Agent::myPort());

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

void TraderAgent::cancelOrder(std::string_view exchange, Order::Side side, std::string_view ticker, int order_id)
{
    CancelOrderMessagePtr msg = std::make_shared<CancelOrderMessage>();
    msg->sender_id = this->agent_id;
    msg->order_id = order_id;
    msg->ticker = std::string{ticker};
    msg->side = side;

    Agent::sendMessageTo(exchange, std::dynamic_pointer_cast<Message>(msg));
}

void TraderAgent::addDelayedStart(int delay_in_seconds)
{
    start_delay_in_seconds_ = delay_in_seconds;
}

void TraderAgent::signalTradingStart()
{
    delay_thread_ = new std::thread([&](){
        if (start_delay_in_seconds_ > 0) 
        {
            std::cout << "Delayed trader start: waiting " << start_delay_in_seconds_ << " to start...\n";
            std::this_thread::sleep_for(std::chrono::seconds(start_delay_in_seconds_));
        }

        std::cout << "Trader starts now.\n";
        onTradingStart();

        std::unique_lock lock{mutex_};
        trading_window_open_ = true;
        lock.unlock();
    });
}