#ifndef EXAMPLE_TRADER_HPP
#define EXAMPLE_TRADER_HPP

#include <iostream>
#include <unordered_set>
#include <boost/asio.hpp>

#include "traderagent.hpp"

class ExampleTrader : public TraderAgent
{
public:

    ExampleTrader(NetworkEntity *network_entity, TraderConfigPtr config)
    : TraderAgent(network_entity, config),
      order_ids_{}
    {
    }

    void onTradingStart() override
    {
        std::cout << "Trading session started.\n";
        is_trading_ = true;
        placeRandomOrder();
    }

    void onTradingEnd() override
    {
        is_trading_ = false;
        std::cout << "Trading session ended.\n";
    }

    void onMarketData(std::string_view exchange, MarketDataMessagePtr msg) override
    {
        // std::cout << "Received market data from " << exchange << ":\n" << msg->summary << "\n";
        if (is_trading_) 
        {
            // Randomly choose to place an order or cancel an order
            if (rand() % 2 == 0)
            {
                placeRandomOrder();
            }
            else
            {
                cancelOldestOrder();
            }
        }
    }

    void onExecutionReport(std::string_view exchange, ExecutionReportMessagePtr msg) override
    {
        std::cout << "Received execution report from " << exchange << ": Order: " << msg->order->id << " Status: " << msg->order->status << "\n";
        if (msg->order->status == Order::Status::FILLED || msg->order->status == Order::Status::CANCELLED)
        {
            order_ids_.erase(msg->order->id);
        }
        else 
        {
            if (!order_ids_.contains(msg->order->id))
            {
                order_ids_.insert(msg->order->id);
            }
        }
    }

    void onCancelReject(std::string_view exchange, CancelRejectMessagePtr msg) override
    {
        std::cout << "CANCEL REJECTED " << exchange << ": Order: " << msg->order_id;
    }

    void placeRandomOrder()
    {
        // Choose a random price between 90 and 110
        double price = 90 + (rand() % 21);
        // Choose random quantity between 1 and 100
        int quantity = 1 + (rand() % 100);

        std::cout << ">> " << Order::Side::BID << " " << quantity << "\n";
        placeLimitOrder("LSE", Order::Side::BID, "AAPL", quantity, price);
    }

    void cancelOldestOrder()
    {
        if (order_ids_.empty()) return;
        
        int order_id = *order_ids_.begin();
        std::cout << ">> " << "CANCEL " << "Id: " << order_id << "\n";
        cancelOrder("LSE", Order::Side::BID, "AAPL", order_id);
    }

private:

    bool is_trading_ = false;

    std::unordered_set<int> order_ids_;
};

#endif