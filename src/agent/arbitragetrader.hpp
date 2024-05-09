#ifndef ARBITRAGE_TRADER_HPP
#define ARBITRAGE_TRADER_HPP

#include <algorithm>
#include <limits>
#include <random>

#include "traderagent.hpp"
#include "../config/arbitrageurconfig.hpp"

class ArbitrageTrader : public TraderAgent
{
public:

    ArbitrageTrader(NetworkEntity *network_entity, ArbitrageurConfigPtr config)
    : TraderAgent(network_entity, config),
      alpha_{config->alpha},
      ticker_{config->ticker},
      cancelling_{config->cancelling},
      trade_interval_ms_{config->trade_interval},
      random_generator_{std::random_device{}()}
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

    /** Gracefully terminates the trader, freeing all memory. */
    void terminate() override
    {
        if (trading_thread_ != nullptr)
        {
            trading_thread_->join();
            delete(trading_thread_);
        }
        TraderAgent::terminate();
    }

    void onTradingStart() override
    {
        std::cout << "Trading window started.\n";
        is_trading_ = true;
        activelyTrade();
    }

    void onTradingEnd() override
    {
        std::unique_lock<std::mutex> lock(mutex_);
        std::cout << "Trading window ended.\n";
        is_trading_ = false;
        lock.unlock();
    }

    void onMarketData(std::string_view exchange, MarketDataMessagePtr msg) override
    {
        // Update market data for the given exchange
        updateMarketData(exchange, msg->data);
        checkForAbitrage();
    }

    void onExecutionReport(std::string_view exchange, ExecutionReportMessagePtr msg) override
    {
        // Order added to order book
        if (msg->order->status == Order::Status::NEW)
        {
            if (msg->order->side == Order::Side::BID)
            {
                last_accepted_bid_id_ = msg->order->id;
            }
            else
            {
                last_accepted_ask_id_ = msg->order->id;
            }
        } 
        else if (msg->order->status == Order::Status::FILLED)
        {
            if (msg->order->side == Order::Side::BID)
            {
                last_accepted_bid_id_ = std::nullopt;
            }
            else
            {
                last_accepted_ask_id_ = std::nullopt;
            }
        }
    }

    void onCancelReject(std::string_view exchange, CancelRejectMessagePtr msg) override
    {

    }

private:

    void activelyTrade()
    {
        trading_thread_ = new std::thread([&, this](){

            std::unique_lock<std::mutex> lock(mutex_);
            while (is_trading_)
            {
                lock.unlock();

                checkForAbitrage();
                sleep();

                lock.lock();
            }
            lock.unlock();

            std::cout << "Finished actively trading.\n";
        });
    }

    void sleep()
    {
        // Generate a random jitter
        std::uniform_real_distribution<> dist(-REL_JITTER, REL_JITTER);
        unsigned long jitter = dist(random_generator_);
        unsigned long sleep_time_ms = std::round(trade_interval_ms_ * (1.0+jitter));

        // Sleep for specified duration
        std::this_thread::sleep_for(std::chrono::milliseconds(sleep_time_ms));
    }

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
            placeArbitrageOrders();
        }
    }

    void placeArbitrageOrders()
    {
        double size = std::min(best_bid_size_, best_ask_size_);
        if (size <= 0) return;

        // Cancel previous orders if not filled
        if (cancelling_ && last_accepted_bid_id_.has_value())
        {
            cancelOrder(best_bid_exchange_, Order::Side::BID, ticker_, last_accepted_bid_id_.value());
            last_accepted_bid_id_ = std::nullopt;
        }
        if (cancelling_ && last_accepted_ask_id_.has_value())
        {   
            cancelOrder(best_ask_exchange_, Order::Side::ASK, ticker_, last_accepted_ask_id_.value());
            last_accepted_ask_id_ = std::nullopt;
        }

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
        placeLimitOrder(best_ask_exchange_, Order::Side::BID, ticker_, size, bid_price, ask_price);
        placeLimitOrder(best_bid_exchange_, Order::Side::ASK, ticker_, size, ask_price, bid_price);
    }

    double best_bid_price_ = std::numeric_limits<double>::min();
    int best_bid_size_ = 0;
    std::string best_bid_exchange_;

    double best_ask_price_ = std::numeric_limits<double>::max();
    int best_ask_size_ = 0;
    std::string best_ask_exchange_;

    double alpha_;
    std::string ticker_;
    bool cancelling_;
    unsigned int trade_interval_ms_;

    std::optional<int> last_accepted_bid_id_ = std::nullopt;
    std::optional<int> last_accepted_ask_id_ = std::nullopt;

    std::mt19937 random_generator_;

    std::mutex mutex_;
    std::thread* trading_thread_ = nullptr;
    bool is_trading_ = false;

    constexpr static double REL_JITTER = 0.25;
};

#endif