#include <iostream>
#include <boost/asio.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/shared_ptr.hpp>

#include <chrono>
#include <thread>

#include "agent/exampletrader.hpp"
#include "agent/stockexchange.hpp"
#include "agent/marketdatawatcher.hpp"
#include "message/message.hpp"
#include "message/messagetype.hpp"
#include "message/market_data_message.hpp"

namespace asio = boost::asio;

int main(int argc, char** argv) {

    if (argc < 4)
    {
        std::cerr << "Usage: " << argv[0] << " exchange <agent_id> <port>" << "\n";
        std::cerr << "       " << argv[0] << " trader <agent_id> <port>" << "\n";
        std::cerr << "       " << argv[0] << " watcher <agent_id> <port>" << "\n";
        return 1;
    }

    std::string agent_type { argv[1] };
    int agent_id { std::stoi(argv[2]) };
    unsigned int port { static_cast<unsigned int>(std::stoi(argv[3])) };

    asio::io_context io_context;
    if (agent_type == "exchange")
    {
        StockExchange exchange{io_context, agent_id, "LSE", 9999};
        exchange.addTradeableAsset("AAPL");

        std::thread t([&](){
            // Allow 30 seconds for connections
            std::this_thread::sleep_for(std::chrono::seconds(10));
            exchange.startTradingSession();
            // Start trading session for 30 seconds
            std::this_thread::sleep_for(std::chrono::seconds(30));
            exchange.endTradingSession();
        });

        exchange.start();
    }
    else if (agent_type == "trader") {
        ExampleTrader trader{io_context, agent_id, port};

        trader.connect("127.0.0.1:9999", "LSE", [&](){
            trader.subscribeToMarket("LSE", "AAPL");
        });
        trader.start();
    }
    else if (agent_type == "watcher") {
        MarketDataWatcher watcher{io_context, agent_id, port};

        watcher.connect("127.0.0.1:9999", "LSE", [&](){
            watcher.subscribeToMarket("LSE", "AAPL");
        });

        watcher.start();
    } else {
        std::cerr << "Invalid agent type: " << agent_type << "\n";
        return 1;
    }

    return 0;
}