#ifndef TRADER_ZIP_HPP
#define TRADER_ZIP_HPP

#include <random>

#include "traderagent.hpp"
#include "../config/zipconfig.hpp"

/** Real-time implementation of the ZIP trading algorithm. */
class TraderZIP : public TraderAgent
{
public:

    TraderZIP(NetworkEntity *network_entity, ZIPConfigPtr config)
    : TraderAgent(network_entity, config),
      exchange_{config->exchange_name},
      ticker_{config->ticker},
      trader_side_{config->side},
      limit_price_{config->limit},
      min_margin_{config->min_margin},
      trade_interval_ms_{config->trade_interval},
      liquidity_interval_ms_{config->liquidity_interval},
      random_generator_{std::random_device{}()},
      mutex_{}
    {
        initialiseConstants();

        // Automatically connect to exchange on initialisation
        connect(config->exchange_addr, config->exchange_name, [=, this](){
            subscribeToMarket(config->exchange_name, config->ticker);
        });

        // Add delayed start
        addDelayedStart(config->delay);
    }

    void onTradingStart() override
    {
        std::cout << "Trading window started.\n";
        next_trade_timestamp_ = timeNow();
        next_undercut_timestamp_ = timeNow() + (liquidity_interval_ms_ * MS_TO_NS);
        is_trading_ = true;
        undercutCompetition();
        placeOrder();
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
        // If trade has occurred always increase margin towards trade price
        // Only decrease margin, if previous order has not executed within the trade interval

        // If trade has occurred
        if (last_market_data_.has_value() && msg->data->cumulative_volume_traded > last_market_data_.value()->cumulative_volume_traded)
        {
            // And trade price went up
            if (msg->data->last_price_traded > last_market_data_.value()->last_price_traded)
            {
                // Raise margin for sellers
                if (trader_side_ == Order::Side::ASK && last_price_ <= msg->data->last_price_traded)
                {
                    double target = increaseTargetPrice(msg->data->last_price_traded);
                    updateMargin(target);
                }
            }
            // And trade price went down
            else if (msg->data->last_price_traded < last_market_data_.value()->last_price_traded)
            {
                // Raise margin for buyers
                if (trader_side_ == Order::Side::BID && last_price_ >= msg->data->last_price_traded)
                {
                    double target = decreaseTargetPrice(msg->data->last_price_traded);
                    updateMargin(target);
                }
            }

            // Lower margin for buyers (conditional)
            if (trader_side_ == Order::Side::BID && timeNow() > next_trade_timestamp_)
            {
                double target = increaseTargetPrice(msg->data->last_price_traded);
                updateMargin(target);
            }
            // Lower margin for sellers (conditional)
            else if (trader_side_ == Order::Side::ASK && timeNow() > next_trade_timestamp_)
            {
                double target = decreaseTargetPrice(msg->data->last_price_traded);
                updateMargin(target);
            }

            // Update last global trade info
            next_undercut_timestamp_ = timeNow() + (liquidity_interval_ms_ * MS_TO_NS);
        }
        else if (!last_market_data_.has_value())
        {
            // Update last global trade info
            next_undercut_timestamp_ = timeNow() + (liquidity_interval_ms_ * MS_TO_NS);
        }

        // Save this market data snapshot
        last_market_data_ = msg->data;

        // Place a new order if it is the time
        placeOrderIfTime();
    }

    void onExecutionReport(std::string_view exchange, ExecutionReportMessagePtr msg) override
    {
        // Order added to order book
        if (msg->order->status == Order::Status::NEW)
        {
            last_accepted_order_id_ = msg->order->id;
        }
        // Order executed successfully
        else if (msg->order->status == Order::Status::FILLED)
        {
            std::cout << "<< Order filled @ " << last_price_ << "\n";
            if (msg->order->client_order_id == last_client_order_id_)
            {
                placeOrder();
            }
        }
    }

    void onCancelReject(std::string_view exchange, CancelRejectMessagePtr msg) override
    {

    }

private:

    void initialiseConstants()
    {
        momentum_ = getRandom(0.0, 0.1);
        learning_rate_ = getRandom(0.0, 0.5);

        if (trader_side_ == Order::Side::BID)
        {
            profit_margin_ = getRandom(0.05, 0.35) * -1.0;
        }
        else
        {
            profit_margin_ = getRandom(0.05, 0.35);
        }

        prev_change_ = 0.0;
        last_price_ = getQuotePrice();
        last_client_order_id_ = 0;
        last_accepted_order_id_ = std::nullopt;
        last_market_data_ = std::nullopt;

        std::cout << "mom=" << momentum_ << "\n";
        std::cout << "lr=" << learning_rate_ << "\n";
        std::cout << "margin=" << profit_margin_ << "\n";
    }

    void undercutCompetition()
    {
        undercut_thread_ = new std::thread([&, this](){

            unsigned long long wait_time;

            std::unique_lock<std::mutex> lock(mutex_);
            while (is_trading_)
            {
                lock.unlock();
                
                // If sufficient time has passed without a trade in the market
                while (timeNow() < next_undercut_timestamp_)
                {
                    wait_time = next_undercut_timestamp_ - timeNow();
                    // std::cout << "Undercut thread sleeping for " << (wait_time / 1e9) << " seconds\n";
                    std::this_thread::sleep_for(std::chrono::nanoseconds(wait_time));
                    // std::cout << "Undercut awake\n";
                }

                // Undercut competition by adjusting the price towards best competing bid/offer
                if (trader_side_ == Order::Side::BID)
                {
                    if (last_market_data_.value()->best_ask_size > 0)
                    {
                        double best_ask = last_market_data_.value()->best_ask;
                        double target = increaseTargetPrice(best_ask);
                        updateMargin(target);
                    }
                }
                else
                {
                    if (last_market_data_.value()->best_bid_size > 0)
                    {
                        double best_bid = last_market_data_.value()->best_bid;
                        double target = decreaseTargetPrice(best_bid);
                        updateMargin(target);
                    }
                }

                placeOrder();
                wait_time = (timeNow() + (trade_interval_ms_ * MS_TO_NS)) - timeNow();
                std::this_thread::sleep_for(std::chrono::nanoseconds(wait_time));
                lock.lock();
            }
            lock.unlock();
        });
    }

    double getQuotePrice()
    {
        double price = round(limit_price_ * (1 + profit_margin_));
        if (trader_side_ == Order::Side::BID)
        {
            price = std::min(limit_price_, price);
        }
        else
        {
            price = std::max(limit_price_, price);
        }
        return price;
    }

    void updateMargin(double target_price)
    {
        double diff = target_price - last_price_;
        double change = ((1.0 - momentum_) * (learning_rate_ * diff)) + (momentum_ * prev_change_);
        
        prev_change_ = change;
        double new_margin = ((last_price_ + change) / limit_price_) - 1.0;

        if (trader_side_ == Order::Side::BID)
        {
            profit_margin_ = std::min(-min_margin_, new_margin);
        }
        else if (trader_side_ == Order::Side::ASK)
        {
            profit_margin_ = std::max(min_margin_, new_margin);
        }

        std::cout << "[Margin Update] Target = " << target_price << " Margin = " << new_margin << " Actual = " << profit_margin_ << "\n";
    }

    double increaseTargetPrice(double price)
    {
        double abs_perturbation = C_A * getRandom(0.0, 1.0);
        double rel_perturbation = (1.0 + (C_R * getRandom(0.0, 1.0))) * price;
        double target = round(abs_perturbation + rel_perturbation);
        return target;
    }

    double decreaseTargetPrice(double price)
    {
        double abs_perturbation = C_A * getRandom(0.0, 1.0);
        double rel_perturbation = (1.0 - (C_R * getRandom(0.0, 1.0))) * price;
        double target = round(rel_perturbation - abs_perturbation);
        return target;
    }

    void placeOrder()
    {
        last_price_ = getQuotePrice();
        placeLimitOrder(exchange_, trader_side_, ticker_, 100, last_price_, Order::TimeInForce::GTC, ++last_client_order_id_);
        next_trade_timestamp_ = timeNow() + (trade_interval_ms_ * MS_TO_NS);
        std::cout << ">> " << (trader_side_ == Order::Side::BID ? "BID" : "ASK") << " " << 100 << " @ " << last_price_ << "\n";
    }

    void placeOrderIfTime()
    {
        if (timeNow() > next_trade_timestamp_)
        {
            /** TODO: Decide if orders should be cancelled. */
            // if (last_accepted_order_id_.has_value())
            // {
            //     double price = getQuotePrice();
            //     if (last_price_ != price)
            //     {
            //         cancelOrder(exchange_, trader_side_, ticker_, last_accepted_order_id_.value());
            //     }
            // }
            // Place a new order with a new price
            placeOrder();
        }
    }

    unsigned long long timeNow()
    {
        std::chrono::system_clock::duration now = std::chrono::system_clock::now().time_since_epoch();
        return std::chrono::duration_cast<std::chrono::nanoseconds>(now).count();
    }

    double getRandom(double lower, double upper)
    {
        std::uniform_real_distribution<> dist(lower, upper);
        return dist(random_generator_);
    }

    std::string exchange_;
    std::string ticker_;
    Order::Side trader_side_;
    double learning_rate_;
    double momentum_;
    double limit_price_;
    double min_margin_;

    unsigned long trade_interval_ms_ = 100;
    unsigned long liquidity_interval_ms_ = 100;

    double profit_margin_;
    double last_price_;
    double prev_change_;

    int last_client_order_id_;
    std::optional<int> last_accepted_order_id_;
    std::optional<MarketDataPtr> last_market_data_;

    unsigned long long next_trade_timestamp_;
    unsigned long long next_undercut_timestamp_;

    std::mt19937 random_generator_;

    std::mutex mutex_;
    std::thread* undercut_thread_;
    bool is_trading_ = false;

    constexpr static double C_A = 0.05;
    constexpr static double C_R = 0.05;

    constexpr static unsigned long MS_TO_NS = 1000000;
};

#endif