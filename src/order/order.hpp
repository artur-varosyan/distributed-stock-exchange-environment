#ifndef ORDER_HPP
#define ORDER_HPP

#include <iostream>
#include <chrono>

#include "trade.hpp"

class Order: std::enable_shared_from_this<Order> {
public:

    enum class Side: int {
        BID,
        ASK
    };

    enum class Status: int {
        NEW,
        FILLED,
        PARTIALLY_FILLED,
        CANCELLED
    };

    Order(int order_id)
    : id{order_id}
    {
        std::chrono::system_clock::duration now = std::chrono::system_clock::now().time_since_epoch();
        timestamp = std::chrono::duration_cast<std::chrono::nanoseconds>(now).count();
    }

    Order(int order_id, int sender_id, std::string_view ticker, Order::Side side, double price, int quantity)
    : id(order_id),
      sender_id{sender_id}, 
      ticker{std::string(ticker)}, 
      side{side}, 
      price{price}, 
      remaining_quantity{quantity},
      cumulative_quantity{0}
    {
        std::chrono::system_clock::duration now = std::chrono::system_clock::now().time_since_epoch();
        timestamp = std::chrono::duration_cast<std::chrono::nanoseconds>(now).count();
    }

    /** Updates the order quantity and price based on the executed trade. */
    void updateOrderWithTrade(TradePtr trade)
    {
        avg_price = ((cumulative_quantity * avg_price) + (trade->quantity * trade->price)) / (cumulative_quantity + trade->quantity);
        cumulative_quantity += trade->quantity;
        remaining_quantity -= trade->quantity;
    }

    /** Returns true if the order is fully filled. */
    bool isFilled() const
    {
        return remaining_quantity == 0;
    }

    int id;
    int sender_id;
    std::string ticker;
    Order::Side side;
    double price;
    double avg_price;
    int remaining_quantity;
    int cumulative_quantity;
    unsigned long long timestamp;

private:

    friend std::ostream& operator<<(std::ostream& os, const Order& order)
    {
        os << order.timestamp << " [Order] Id: " << order.id << " Trader: " << order.sender_id << " " << order.ticker << " " << order.side << " " << order.remaining_quantity << " @ $" << order.price;
        return os;
    }

    friend std::ostream& operator<<(std::ostream& os, const Order::Status& status)
    {
        switch (status)
        {
            case Order::Status::NEW:
                os << "NEW";
                break;
            case Order::Status::FILLED:
                os << "FILLED";
                break;
            case Order::Status::PARTIALLY_FILLED:
                os << "PARTIALLY FILLED";
                break;
            case Order::Status::CANCELLED:
                os << "CANCELLED";
                break;
        }
        return os;
    
    }

    friend std::ostream& operator<<(std::ostream& os, const Order::Side& side)
    {
        switch (side)
        {
            case Order::Side::BID:
                os << "BID";
                break;
            case Order::Side::ASK:
                os << "ASK";
                break;
        }
        return os;
    }


};

typedef std::shared_ptr<Order> OrderPtr;

#endif