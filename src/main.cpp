#include <iostream>
#include <chrono>
#include <thread>

#include <boost/asio.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/program_options.hpp>

#include "agent/exampletrader.hpp"
#include "agent/stockexchange.hpp"
#include "agent/marketdatawatcher.hpp"
#include "agent/traderzic.hpp"
#include "message/message.hpp"
#include "message/messagetype.hpp"
#include "message/market_data_message.hpp"

namespace asio = boost::asio;
namespace po = boost::program_options;

std::string showUsage() {
    std::stringstream ss;
    ss << "Usage: " << "./simulation" << " <agent> <agent_id> [options]" << "\n\n";
    ss << "Agents:\n";
    ss << "  " << "exchange" << "\t" << "multithreaded stock exchange implementation" << "\n";
    ss << "  " << "watcher" << "\t" << "live market data watcher" << "\n";
    ss << "  " << "zic" << "\t\t" << "zero intelligence constrained trader" << "\n";
    ss << "\n";
    return ss.str();
}

int main(int argc, char** argv) {

    po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "show help message")
        ("port", po::value<unsigned int>()->default_value(8080), "set the port of the current agent")
        ("ticker", po::value<std::string>()->default_value(std::string{"AAPL"}), "set the ticker to trade")
        ("exchange-name", po::value<std::string>()->default_value(std::string{"LSE"}), "set the name of the exchange")
        ("connect-time", po::value<int>()->default_value(30), "(exchange only) the time allowed for traders to connect (seconds)")
        ("trading-time", po::value<int>()->default_value(60), "(exchange only) the time of the trading window (seconds)")
        ("delay", po::value<unsigned int>()->default_value(0), "(trader only) delayed start for trader (seconds)")
        ("side", po::value<std::string>()->default_value(std::string{"buyer"}), "(trader only) set the trader side: buyer or seller")
        ("limit", po::value<double>()->default_value(100), "(trader only) set the limit price of the trader")
        ("exchange-addr", po::value<std::string>()->default_value(std::string{"127.0.0.1:9999"}), "(trader only) set the IPv4 address of the exchange")
    ;

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (argc < 3 || vm.count("help"))
    {
        std::cout << "\n" << showUsage() << desc << std::endl;
        exit(1);
    }

    std::string agent_type { argv[1] };
    int agent_id { std::stoi(argv[2]) };
    unsigned int port { vm["port"].as<unsigned int>() };

    asio::io_context io_context;
    if (agent_type == "exchange")
    {
        // Get configuration
        std::string exchange_name { vm["exchange-name"].as<std::string>() };
        std::string ticker { vm["ticker"].as<std::string>() };
        int connect_time { vm["connect-time"].as<int>() };
        int trading_time { vm["trading-time"].as<int>() };

        StockExchange exchange{io_context, agent_id, exchange_name, port};
        exchange.addTradeableAsset(ticker);
        exchange.setTradingWindow(connect_time, trading_time);
        exchange.start();
    }
    else if (agent_type == "trader") {

        // Get configuration
        std::string exchange_name { vm["exchange-name"].as<std::string>() };
        std::string exchange_addr { vm["exchange-addr"].as<std::string>() };
        std::string ticker { vm["ticker"].as<std::string>() };

        ExampleTrader trader{io_context, agent_id, port};
        trader.connect(exchange_addr, exchange_name, [&](){
            trader.subscribeToMarket(exchange_name, ticker);
        });
        trader.start();
    }
    else if (agent_type == "watcher") {

        // Get configuration
        std::string exchange_name { vm["exchange-name"].as<std::string>() };
        std::string exchange_addr { vm["exchange-addr"].as<std::string>() };
        std::string ticker { vm["ticker"].as<std::string>() };

        MarketDataWatcher watcher{io_context, agent_id, port};

        watcher.connect(exchange_addr, exchange_name, [&](){
            watcher.subscribeToMarket(exchange_name, ticker);
        });
        watcher.start();
    } 
    else if (agent_type == "zic") 
    {
        // Get configuration
        std::string exchange_name { vm["exchange-name"].as<std::string>() };
        std::string exchange_addr { vm["exchange-addr"].as<std::string>() };
        std::string ticker { vm["ticker"].as<std::string>() };

        Order::Side trader_side = (vm["side"].as<std::string>() == "buyer") ? Order::Side::BID : Order::Side::ASK;
        double limit { vm["limit"].as<double>() };

        TraderZIC trader{io_context, agent_id, port, exchange_name, ticker, trader_side, limit};

        unsigned int delay { vm["delay"].as<unsigned int>() };
        trader.addDelayedStart(delay);

        trader.connect(exchange_addr, exchange_name, [&](){
            trader.subscribeToMarket(exchange_name, ticker);
        });
        trader.start();
    } 
    else {
        std::cerr << "Invalid agent type: " << agent_type << "\n";
        std::cout << "\n" << showUsage() << desc << std::endl;
        exit(1);
    }

    return 0;
}