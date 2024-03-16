#ifndef ORDERBOOK_HPP
#define ORDERBOOK_HPP

#include <iostream>
#include <string>
#include <chrono>
#include <queue>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/string.hpp>

#include "order.hpp"
#include "orderqueue.hpp"

class OrderBook;
typedef std::shared_ptr<OrderBook> OrderBookPtr;

/** The order book storing all orders for a single ticker. */
class OrderBook : std::enable_shared_from_this<OrderBook> {
public:

    /** The level 1 summary of the current state of the order book. */
    class Summary {
        public:
            Summary() = default;

            std::string ticker;
            double best_bid;
            double best_ask;
            double last_price;
            int bid_size;
            int ask_size;
            int last_quantity_traded;
            unsigned long long timestamp;

        private:
            friend std::ostream& operator<<(std::ostream& os, const Summary& summary)
            {
                os << "Summary " << summary.ticker << ":\n" 
                << "LAST TRADE: " << summary.last_quantity_traded << " @ $" << summary.last_price << "\n"
                << "BEST BID: " << summary.best_bid << "\n" 
                << "BEST ASK: " << summary.best_ask << "\n" 
                << "BID SIZE: " << summary.bid_size << "\n" 
                << "ASK SIZE: " << summary.ask_size << "\n";
                return os;
            }

            friend class boost::serialization::access;
            template<class Archive>
            void serialize(Archive & ar, const unsigned int version)
            {
                ar & ticker;
                ar & best_bid;
                ar & best_ask;
                ar & bid_size;
                ar & ask_size;
                ar & last_price;
                ar & last_quantity_traded;
                ar & timestamp;
            }
    };


    OrderBook(std::string_view ticker)
    : ticker_{std::string(ticker)},
      bids_{Order::Side::BID},
      asks_{Order::Side::ASK},
      bid_size_{0},
      ask_size_{0}
    {
    }
    
    /** Adds the given order to the order book. */
    void addOrder(OrderPtr order);

    /** Removes the given order from the order book if exists. Returns false if order not present. */
    bool removeOrder(int order_id, Order::Side side);

    /** Returns the best bid in the order book. */
    std::optional<OrderPtr> bestBid();

    /** Returns the best ask in the order book. */
    std::optional<OrderPtr> bestAsk();

    /** Removes the best bid from the order queue. */
    void popBestBid();

    /** Removes the best ask from the order queue. */
    void popBestAsk();

    /** Checks if the given order exists in the order book. */
    bool contains(int order_id, Order::Side side);

    /** Returns the summary of the current state of the order book. */
    Summary getSummary();

    /** Creates a new order book for the given ticker. */
    static OrderBookPtr create(std::string_view ticker)
    {
        return std::make_shared<OrderBook>(ticker);
    }

private:

    std::string ticker_;
    OrderQueue bids_;
    OrderQueue asks_;

    int bid_size_;
    int ask_size_;
};

#endif