#ifndef ORDERBOOK_HPP
#define ORDERBOOK_HPP

#include <iostream>
#include <string>
#include <chrono>
#include <queue>
#include <unordered_map>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/string.hpp>

#include "order.hpp"
#include "limitorder.hpp"
#include "orderqueue.hpp"
#include "trade.hpp"

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
            int best_bid_size;
            int best_ask_size;

            int bids_volume;
            int asks_volume;
            int bids_count;
            int asks_count;

            double last_price_traded;
            int last_quantity_traded;

            double high_price;
            double low_price;
            int cumulative_volume_traded;
            int trades_count;

            unsigned long long timestamp;

        private:
            friend std::ostream& operator<<(std::ostream& os, const Summary& summary)
            {
                os << "Summary " << summary.ticker << ":\n" 
                << "LAST TRADE: " << summary.last_quantity_traded << " @ $" << summary.last_price_traded << "\n"
                << "BEST BID: " << summary.best_bid_size << " @ $" << summary.best_bid << "\n" 
                << "BEST ASK: " << summary.best_bid_size << " @ $" << summary.best_ask << "\n" 
                << "BID VOL: " << summary.bids_volume << "\n" 
                << "ASK VOL: " << summary.asks_volume << "\n"
                << "BID ORDERS: " << summary.bids_count << "\n" 
                << "ASK ORDERS: " << summary.asks_count << "\n"
                << "VOLUME TRADED: " << summary.cumulative_volume_traded << "\n"
                << "HIGH: " << summary.high_price << "\n"
                << "LOW: " << summary.low_price << "\n"
                << "TRADES: " << summary.trades_count << "\n";
                return os;
            }

            friend class boost::serialization::access;
            template<class Archive>
            void serialize(Archive & ar, const unsigned int version)
            {
                ar & ticker;
                ar & best_bid;
                ar & best_ask;
                ar & best_bid_size;
                ar & best_ask_size;

                ar & bids_volume;
                ar & asks_volume;
                ar & bids_count;
                ar & asks_count;

                ar & last_price_traded;
                ar & last_quantity_traded;

                ar & high_price;
                ar & low_price;
                ar & cumulative_volume_traded;
                ar & trades_count;

                ar & timestamp;
            }
    };


    OrderBook(std::string_view ticker)
    : ticker_{std::string(ticker)},
      bids_{Order::Side::BID},
      asks_{Order::Side::ASK},
      bids_volume_{0},
      asks_volume_{0},
      order_count_{0},
      trade_volume_{0},
      trade_count_{0}
    {
    }
    
    /** Adds the given order to the order book. */
    void addOrder(LimitOrderPtr order);

    /** Removes the given order from the order book if exists. Returns nullopt if order does not exist. */
    std::optional<LimitOrderPtr> removeOrder(int order_id, Order::Side side);

    /** Returns the best bid in the order book. */
    std::optional<LimitOrderPtr> bestBid();

    /** Returns the best ask in the order book. */
    std::optional<LimitOrderPtr> bestAsk();

    /** Removes the best bid from the order queue. */
    void popBestBid();

    /** Removes the best ask from the order queue. */
    void popBestAsk();

    /** Checks if the given order exists in the order book. */
    bool contains(int order_id, Order::Side side);

    /** Logs the details of the executed trade for statistics. */
    void logTrade(TradePtr trade);

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

    int bids_volume_;
    int asks_volume_;
    int order_count_;

    std::optional<TradePtr> last_trade_;

    std::optional<double> trade_high_;
    std::optional<double> trade_low_;
    int trade_volume_;
    int trade_count_;
};

#endif