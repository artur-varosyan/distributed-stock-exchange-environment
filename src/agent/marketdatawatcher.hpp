#ifndef MARKET_DATA_WATCHER_HPP
#define MARKET_DATA_WATCHER_HPP

#include "../config/marketwatcherconfig.hpp"

class MarketDataWatcher : public Agent
{
public:

    MarketDataWatcher(NetworkEntity* network_entity, MarketWatcherConfigPtr config)
    : Agent(network_entity, config)
    {
        // Automatically connect to exchange on initialisation
        connect(config->exchange_addr, config->exchange_name, [=, this](){
            subscribeToMarket(config->exchange_name, config->ticker);
        });
    };

    /** Prints the market data to the standard output. */
    void onMarketData(std::string_view sender, MarketDataMessagePtr msg)
    {
        // Clear the screen
        std::cout << "\033[2J\033[1;1H";
        // Print the market data
        std::cout << sender << ":\n" << *msg->data << "\n";
    }

    /** Subscribe to market data from the given market. */
    void subscribeToMarket(std::string_view exchange, std::string_view ticker)
    {
        SubscribeMessagePtr msg = std::make_shared<SubscribeMessage>();
        msg->ticker = std::string{ticker};
        msg->address = myAddr() + std::string{":"} + std::to_string(myPort());

        Agent::sendMessageTo(exchange, std::dynamic_pointer_cast<Message>(msg));
    }

private:

    /** Checks the type of the incoming message and makes a callback. */
    std::optional<MessagePtr> handleMessageFrom(std::string_view sender, MessagePtr message) override
    {
        std::cout << "Market Data Watcher received a message" << std::endl;
        return std::nullopt;
    }

    /** Checks the type of the incoming broadcast and makes a callback. */
    void handleBroadcastFrom(std::string_view sender, MessagePtr message) override
    {
        if (message->type == MessageType::MARKET_DATA)
        {
            onMarketData(sender, std::dynamic_pointer_cast<MarketDataMessage>(message));
        }
    }

};

#endif