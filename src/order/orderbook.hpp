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
#include "../trade/trade.hpp"
#include "../trade/marketdata.hpp"

class OrderBook;
typedef std::shared_ptr<OrderBook> OrderBookPtr;

/** The order book storing all orders for a single ticker. */
class OrderBook : std::enable_shared_from_this<OrderBook> {
public:

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

    /** Updates the order quantity and price based on the executed trade. */
    void updateOrderWithTrade(OrderPtr order, TradePtr trade);

    /** Returns the best bid in the order book. */
    std::optional<LimitOrderPtr> bestBid();

    /** Returns the best ask in the order book. */
    std::optional<LimitOrderPtr> bestAsk();

    /** Returns the aggregate size of all bids at the best bid price. */
    int bestBidSize();

    /** Returns the aggregate size of all asks at the best ask price. */
    int bestAskSize();

    /** Removes the best bid from the order queue. */
    void popBestBid();

    /** Removes the best ask from the order queue. */
    void popBestAsk();

    /** Checks if the given order exists in the order book. */
    bool contains(int order_id, Order::Side side);

    /** Logs the details of the executed trade for statistics. */
    void logTrade(TradePtr trade);

    /** Returns live level 1 market data. */
    MarketData getLiveMarketData();

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

    std::unordered_map<double, int> bids_sizes_;
    std::unordered_map<double, int> asks_sizes_;

    std::optional<TradePtr> last_trade_;

    std::optional<double> trade_high_;
    std::optional<double> trade_low_;
    int trade_volume_;
    int trade_count_;
};

#endif