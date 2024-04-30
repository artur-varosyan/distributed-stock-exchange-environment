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
      limit_price_{config->limit},
      random_generator_{std::random_device{}()}
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
        my_trade_deadline_ = timeNow();
        global_trade_deadline_ = timeNow() + (GLOBAL_TRADE_WAIT_TIME_IN_MS * MS_TO_NS);
        // undercutCompetition();
        placeOrder();
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
        if (last_market_data_.has_value() && msg->data->cumulative_volume_traded > last_market_data_.value()->cumulative_volume_traded)
        {
            // Trade price went up
            if (msg->data->last_price_traded > last_market_data_.value()->last_price_traded)
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
            global_trade_deadline_ = timeNow() + (GLOBAL_TRADE_WAIT_TIME_IN_MS * MS_TO_NS);
        }
        else if (!last_market_data_.has_value())
        {
            // Update last global trade info
            global_trade_deadline_ = timeNow() + (GLOBAL_TRADE_WAIT_TIME_IN_MS * MS_TO_NS);
        }

        // Save this market data snapshot
        last_market_data_ = msg->data;

        // Place a new order if it is the time
        placeOrderIfTime();
    }

    void onExecutionReport(std::string_view exchange, ExecutionReportMessagePtr msg) override
    {
        std::cout << "id=" << msg->order->id << " status=" << msg->order->status << " rem_qty=" << msg->order->remaining_quantity << " trade_nonnull=" << msg->trade << "\n";

        // Order added to order book
        if (msg->order->status == Order::Status::NEW)
        {
            last_accepted_order_id_ = msg->order->id;
        }
        // Order executed successfully
        else if (msg->order->status == Order::Status::FILLED)
        {
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
            while (true)
            {
                // If sufficient time has passed without a trade in the market
                while (timeNow() < global_trade_deadline_)
                {
                    wait_time = global_trade_deadline_ - timeNow();
                    std::cout << "Undercut thread sleeping for " << (wait_time / 1e9) << " seconds\n";
                    std::this_thread::sleep_for(std::chrono::nanoseconds(wait_time));
                    std::cout << "Undercut awake\n";
                }

                std::cout << "No trades in market, undercutting competition\n";

                // Undercut competition by adjusting the price towards best competing bid/offer
                if (trader_side_ == Order::Side::BID)
                {
                    double best_ask = last_market_data_.value()->best_ask;
                    double target = increaseTargetPrice(best_ask);
                    std::cout << "Target price of " << target << "\n";
                    updateMargin(target);
                }
                else
                {
                    double best_bid = last_market_data_.value()->best_bid;
                    double target = decreaseTargetPrice(best_bid);
                    std::cout << "Target price of " << target << "\n";
                    updateMargin(target);
                }

                placeOrder();
                wait_time = (timeNow() + (MY_TRADE_WAIT_TIME_IN_MS * MS_TO_NS)) - timeNow();
                std::this_thread::sleep_for(std::chrono::nanoseconds(wait_time));
            }
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
        profit_margin_ = ((last_price_ + change) / limit_price_) - 1.0;
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
        my_trade_deadline_ = timeNow() + (MY_TRADE_WAIT_TIME_IN_MS * MS_TO_NS);
        std::cout << ">> " << (trader_side_ == Order::Side::BID ? "BID" : "ASK") << " " << 100 << " @ " << last_price_ << "\n";
    }

    void placeOrderIfTime()
    {
        if (timeNow() > my_trade_deadline_)
        {
            // Cancel last order
            if (last_accepted_order_id_.has_value())
            {
                cancelOrder(exchange_, trader_side_, ticker_, last_accepted_order_id_.value());
            }
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

    
    std::mt19937 random_generator_;

    std::string exchange_;
    std::string ticker_;
    Order::Side trader_side_;

    int last_client_order_id_;
    std::optional<int> last_accepted_order_id_;
    double limit_price_;
    double profit_margin_;
    double last_price_;
    double prev_change_;

    double learning_rate_;
    double momentum_;

    std::optional<MarketDataPtr> last_market_data_;
    double last_trade_price_;

    unsigned long long my_trade_deadline_;
    unsigned long long global_trade_deadline_;

    // bool trading_window_open = false;
    std::mutex mutex_;
    std::thread* undercut_thread_ = nullptr;

    constexpr static double C_A = 0.05;
    constexpr static double C_R = 0.05;

    constexpr static double MIN_PRICE = 1.0;
    constexpr static double MAX_PRICE = 200.0;

    constexpr static unsigned long MS_TO_NS = 1000000;
    constexpr static unsigned long MY_TRADE_WAIT_TIME_IN_MS = 100;
    constexpr static unsigned long GLOBAL_TRADE_WAIT_TIME_IN_MS = 500;

};

#endif