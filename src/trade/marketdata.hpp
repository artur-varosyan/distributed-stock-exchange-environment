#ifndef MARKET_DATA_HPP
#define MARKET_DATA_HPP

#include <string>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/string.hpp>

#include "../utilities/csvprintable.hpp"

/** The Level 1 Market Data Feed **/
class MarketData : public CSVPrintable, std::enable_shared_from_this<MarketData> {
    public:
        MarketData() = default;

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

        std::string describeCSVHeaders() const override
        {
            return "timestamp,ticker,best_bid,best_ask,best_bid_size,best_ask_size,bids_volume,asks_volume,bids_count,asks_count";
        }

        std::string toCSV() const override
        {
            return std::to_string(timestamp) + "," + ticker + "," + std::to_string(best_bid) + "," + std::to_string(best_ask) + "," + std::to_string(best_bid_size) + "," + std::to_string(best_ask_size) + "," + std::to_string(bids_volume) + "," + std::to_string(asks_volume) + "," + std::to_string(bids_count) + "," + std::to_string(asks_count);
        }

    private:
        friend std::ostream& operator<<(std::ostream& os, const MarketData& data)
        {
            os << "Market Data: " << data.ticker << ":\n" 
            << "LAST TRADE: " << data.last_quantity_traded << " @ $" << data.last_price_traded << "\n"
            << "BEST BID: " << data.best_bid_size << " @ $" << data.best_bid << "\n" 
            << "BEST ASK: " << data.best_bid_size << " @ $" << data.best_ask << "\n" 
            << "BID VOL: " << data.bids_volume << "\n" 
            << "ASK VOL: " << data.asks_volume << "\n"
            << "BID ORDERS: " << data.bids_count << "\n" 
            << "ASK ORDERS: " << data.asks_count << "\n"
            << "VOLUME TRADED: " << data.cumulative_volume_traded << "\n"
            << "HIGH: " << data.high_price << "\n"
            << "LOW: " << data.low_price << "\n"
            << "TRADES: " << data.trades_count << "\n";
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

typedef std::shared_ptr<MarketData> MarketDataPtr;

#endif