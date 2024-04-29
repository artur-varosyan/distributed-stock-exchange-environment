#ifndef ORDER_HPP
#define ORDER_HPP

#include <iostream>
#include <chrono>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/shared_ptr.hpp>

#include "../trade/trade.hpp"

class OrderFactory;

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

    enum class Type: int {
        MARKET,
        LIMIT
    };

    enum class TimeInForce: int {
        GTC,                         // Good-Til-Cancelled
        IOC,                         // Immediate-Or-Cancel
        FOK                          // Fill-Or-Kill
    };

    Order() {};

    Order(int order_id, Type type)
    : id{order_id},
      type{type}
    {
        std::chrono::system_clock::duration now = std::chrono::system_clock::now().time_since_epoch();
        timestamp = std::chrono::duration_cast<std::chrono::nanoseconds>(now).count();
    }

    Order(int order_id, Type type, TimeInForce time_in_force)
    : id{order_id},
      type{type},
      time_in_force{time_in_force}
    {
        std::chrono::system_clock::duration now = std::chrono::system_clock::now().time_since_epoch();
        timestamp = std::chrono::duration_cast<std::chrono::nanoseconds>(now).count();
    }

    virtual ~Order() = default;

    bool isFilled() const
    {
        return remaining_quantity <= 0;
    }

    void setStatus(Order::Status new_status)
    {
        status = new_status;
    }

    int id;
    int client_order_id;
    int sender_id;
    Order::Type type;
    Order::TimeInForce time_in_force;
    std::string ticker;
    Order::Side side;
    Order::Status status;
    double avg_price;
    int remaining_quantity;
    int cumulative_quantity;
    unsigned long long timestamp;

protected:

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

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & id;
        ar & client_order_id;
        ar & type;
        ar & time_in_force;
        ar & ticker;
        ar & status;
        ar & side;
        ar & avg_price;
        ar & remaining_quantity;
        ar & cumulative_quantity;
        ar & timestamp;
    }
};

typedef std::shared_ptr<Order> OrderPtr;

#endif