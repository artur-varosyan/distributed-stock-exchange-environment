#ifndef ORDER_HPP
#define ORDER_HPP

#include <iostream>
#include <chrono>

class Order: std::enable_shared_from_this<Order> {
public:

    enum class Side {
        BID,
        ASK
    };

    Order(int order_id)
    : id{order_id}
    {
        timestamp = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    }

    Order(int order_id, int sender_id, std::string_view ticker, Order::Side side, double price, int quantity)
    : id(order_id),
      sender_id{sender_id}, 
      ticker{std::string(ticker)}, 
      side{side}, 
      price{price}, 
      quantity{quantity}
    {
        timestamp = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    }

    int id;
    int sender_id;
    std::string ticker;
    Order::Side side;
    double price;
    int quantity;
    unsigned long long timestamp;

private:

    friend std::ostream& operator<<(std::ostream& os, const Order& order)
    {
        os << "Order: " << order.sender_id << " " << order.ticker << " " << (order.side == Order::Side::BID ? "BID" : "ASK") << " " << order.price << " " << order.quantity;
        return os;
    }

};

typedef std::shared_ptr<Order> OrderPtr;

#endif