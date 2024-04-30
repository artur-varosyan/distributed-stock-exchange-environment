#ifndef ARBITRAGE_TRADER_HPP
#define ARBITRAGE_TRADER_HPP

#include <algorithm>
#include <limits>

#include "traderagent.hpp"
#include "../config/arbitrageurconfig.hpp"

class ArbitrageTrader : public TraderAgent
{
public:

    ArbitrageTrader(NetworkEntity *network_entity, ArbitrageurConfigPtr config)
    : TraderAgent(network_entity, config),
      alpha_{config->alpha},
      ticker_{config->ticker}
    {
        /** TODO: Consider changing config to allow arbitrageur to trade on abitrary number of exchanges. */

        // Connect to both exchanges
        connect(config->exchange0_addr, config->exchange0_name, [=, this](){
            subscribeToMarket(config->exchange0_name, config->ticker);
        });

        connect(config->exchange1_addr, config->exchange1_name, [=, this](){
            subscribeToMarket(config->exchange1_name, config->ticker);
        });

        // Add delayed start
        addDelayedStart(config->delay);
    }

    void onTradingStart() override
    {
        std::cout << "Arbitrageur received trading started signal" << "\n";
    }

    void onTradingEnd() override
    {

    }

    void onMarketData(std::string_view exchange, MarketDataMessagePtr msg) override
    {
        // Update market data for the given exchange
        updateMarketData(exchange, msg->data);
        checkForAbitrage();
    }

    void onExecutionReport(std::string_view exchange, ExecutionReportMessagePtr msg) override
    {

    }

    void onCancelReject(std::string_view exchange, CancelRejectMessagePtr msg) override
    {

    }

private:

    void updateMarketData(std::string_view exchange, MarketDataPtr data)
    {
        // Update best known ask (ovewrite best ask if was from this exchange)
        if (best_ask_exchange_ == std::string{exchange} || data->best_ask < best_ask_price_)
        {
            best_ask_price_ = data->best_ask;
            best_ask_size_ = data->best_ask_size;
            best_ask_exchange_ = std::string{exchange};
        }

        // Update best known bid (ovewrite best bid if was from this exchange)
        if (best_bid_exchange_ == std::string{exchange} || data->best_bid > best_bid_price_)
        {
            best_bid_price_ = data->best_bid;
            best_bid_size_ = data->best_bid_size;
            best_bid_exchange_ = std::string{exchange};
        }
    }

    void checkForAbitrage()
    {
        // Check for sufficient arbitrage opportunity
        if ((best_bid_exchange_ != best_ask_exchange_) && (best_bid_price_ > (1 + alpha_) * best_ask_price_))
        {
            // Execute arbitrage
            placeArbitrageOrders();
        }
    }

    void placeArbitrageOrders()
    {
        double size = std::min(best_bid_size_, best_ask_size_);
        double midpoint = (best_bid_price_ + best_ask_price_) / 2;

        double bid_price = std::floor(midpoint);
        double ask_price = std::ceil(midpoint);

        std::cout << "[Opportunity] " 
        << "BEST BID: " << best_bid_exchange_ << " @ " << best_bid_price_ << " "
        << "BEST ASK: " << best_ask_exchange_ << " @ " << best_ask_price_ << std::endl;
        
        std::cout << "[Arbitrage] " 
        << "BID: " << best_ask_exchange_ << " @ " <<  bid_price 
        << " ASK: " << best_bid_exchange_ << " @ " << ask_price << std::endl;

        // Note: Arbitrageur places bid order on the exchange with best ask and vice versa
        placeLimitOrder(best_ask_exchange_, Order::Side::BID, ticker_, size, bid_price, best_ask_price_);
        placeLimitOrder(best_bid_exchange_, Order::Side::ASK, ticker_, size, ask_price,  best_bid_price_);
    }

    double best_bid_price_ = std::numeric_limits<double>::min();
    int best_bid_size_ = 0;
    std::string best_bid_exchange_;

    double best_ask_price_ = std::numeric_limits<double>::max();
    int best_ask_size_ = 0;
    std::string best_ask_exchange_;

    double alpha_;
    std::string ticker_;
};

#endif