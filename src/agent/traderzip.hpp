#ifndef TRADER_ZIP_HPP
#define TRADER_ZIP_HPP

#include <random>

#include "traderagent.hpp"

/** Prototype ZIP trader implementation. */
class TraderZIP : public TraderAgent
{
public:

    TraderZIP(NetworkEntity *network_entity, TraderConfigPtr config)
    : TraderAgent(network_entity, config),
      exchange_{config->exchange_name},
      ticker_{config->ticker},
      trader_side_{config->side},
      limit_price_{config->limit}
    {
        initialiseConstants();
    }

    void onTradingStart() override
    {
        std::cout << "Trading window started.\n";
    }

    void onTradingEnd() override
    {
        std::cout << "Trading window ended.\n";
    }

    void onMarketData(std::string_view exchange, MarketDataMessagePtr msg) override
    {
        // If trade has occurred always increase margin towards trade price
        // Only decrease margin towards trade price, if have not traded in a while

        // If trade has occurred at a different price
        if (msg->data->last_price_traded != last_trade_price_)
        {
            // Trade price went up
            if (msg->data->last_price_traded > last_trade_price_)
            {
                // Raise margin for sellers
                if (trader_side_ == Order::Side::ASK)
                {
                    double target = increaseTargetPrice(msg->data->last_price_traded);
                    updateMargin(target);
                }
                // Lower margin for buyers (conditional)
                else if (trader_side_ == Order::Side::BID && timeNow() > my_trade_deadline_)
                {
                    double target = increaseTargetPrice(msg->data->last_price_traded);
                    updateMargin(target);
                }
            }
            // Trade price went down
            else
            {
                // Raise margin for buyers
                if (trader_side_ == Order::Side::BID)
                {
                    double target = decreaseTargetPrice(msg->data->last_price_traded);
                    updateMargin(target);
                }
                // Lower margin for sellers (conditional)
                else if (trader_side_ == Order::Side::ASK && timeNow() > my_trade_deadline_)
                {
                    double target = decreaseTargetPrice(msg->data->last_price_traded);
                    updateMargin(target);
                }
            }

            // Update last global trade info
            last_trade_price_ = msg->data->last_price_traded;
            global_trade_deadline_ = timeNow() + (GLOBAL_TRADE_WAIT_TIME_IN_MS * MS_TO_NS);
        }
        else
        {
            // If sufficient time has passed
            if (timeNow() < global_trade_deadline_) return;

            // Undercut competition by adjusting the price towards best competing bid/offer
            if (trader_side_ == Order::Side::BID)
            {
                double best_ask = msg->data->best_ask;
                if (price_ < best_ask)
                {
                    double target = decreaseTargetPrice(best_ask);
                    updateMargin(target);
                }
            }
            else
            {
                double best_bid = msg->data->best_bid;
                if (price_ > best_bid)
                {
                    double target = increaseTargetPrice(best_bid);
                    updateMargin(target);
                }
            }
        }

        // Place a new order if it is the time
        placeOrderIfTime();
    }

    void onExecutionReport(std::string_view exchange, ExecutionReportMessagePtr msg) override
    {
        // Order added to order book
        if (msg->order->status == Order::Status::NEW)
        {
            last_order_id_ = msg->order->id;
        }
        // Order executed successfully
        else if (msg->order->isFilled())
        {
            // Place a new order
            placeLimitOrder(exchange_, trader_side_, ticker_, 100, getQuotePrice());
            my_trade_deadline_ = timeNow() + (MY_TRADE_WAIT_TIME_IN_MS * MS_TO_NS);
        }
    }

    void onCancelReject(std::string_view exchange, CancelRejectMessagePtr msg) override
    {

    }

private:

    void initialiseConstants()
    {
        momentum_ = 0.1 * rand();
        learning_rate_ = 0.1 + (0.4 * rand());

        if (trader_side_ == Order::Side::BID)
        {
            profit_margin_ = -1.0 * (0.05 + (0.3 * rand()));
        }
        else
        {
            profit_margin_ = 0.05 + (0.3 * rand());
        }
        
        my_trade_deadline_ = timeNow() + (MY_TRADE_WAIT_TIME_IN_MS * MS_TO_NS);
        global_trade_deadline_ = timeNow() + (GLOBAL_TRADE_WAIT_TIME_IN_MS * MS_TO_NS);
    }

    double getQuotePrice()
    {
        return round(limit_price_ * (1 + profit_margin_));
    }

    double updateMargin(double target_price)
    {
        double diff = target_price - price_;
        double change = ((1.0 - momentum_) * (learning_rate_ * diff)) + (momentum_ * prev_change_);
        prev_change_ = change;
        profit_margin_ = ((price_ + change) / limit_price_) - 1.0;
    }

    double increaseTargetPrice(double price)
    {
        double abs_perturbation = C_A * rand();
        double rel_perturbation = (1.0 + (C_R * rand())) * price;
        double target = round(abs_perturbation + rel_perturbation);
        return target;
    }

    double decreaseTargetPrice(double price)
    {
        double abs_perturbation = C_A * rand();
        double rel_perturbation = (1.0 - (C_R * rand())) * price;
        double target = round(rel_perturbation - abs_perturbation);
        return target;
    }

    void placeOrderIfTime()
    {
        if (timeNow() > my_trade_deadline_)
        {
            // Cancel last order
            if (last_order_id_.has_value())
            {
                cancelOrder(exchange_, trader_side_, ticker_, last_order_id_.value());
            }

            // Place a new order
            placeLimitOrder(exchange_, trader_side_, ticker_, 100, getQuotePrice());
        }
    }

    unsigned long long timeNow()
    {
        std::chrono::system_clock::duration now = std::chrono::system_clock::now().time_since_epoch();
        return std::chrono::duration_cast<std::chrono::nanoseconds>(now).count();
    }


    std::string exchange_;
    std::string ticker_;
    Order::Side trader_side_;

    std::optional<int> last_order_id_;
    double limit_price_;
    double profit_margin_;
    double price_;
    double prev_change_;

    double learning_rate_;
    double momentum_;

    unsigned long long last_trade_time_;
    double last_trade_price_;

    unsigned long long my_trade_deadline_;
    unsigned long long global_trade_deadline_;

    std::mt19937 random_generator_;

    constexpr static double C_A = 0.05;
    constexpr static double C_R = 0.05;

    constexpr static double MIN_PRICE = 1.0;
    constexpr static double MAX_PRICE = 200.0;

    constexpr static unsigned long MS_TO_NS = 1000;
    constexpr static unsigned long MY_TRADE_WAIT_TIME_IN_MS = 500;
    constexpr static unsigned long GLOBAL_TRADE_WAIT_TIME_IN_MS = 1000;

};

#endif