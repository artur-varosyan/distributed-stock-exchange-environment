#include <iostream>
#include <boost/asio.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/shared_ptr.hpp>

#include <chrono>
#include <thread>

#include "agent/exampletrader.hpp"
#include "agent/exchangeagent.hpp"
#include "message/message.hpp"
#include "message/messagetype.hpp"
#include "message/market_data_message.hpp"

namespace asio = boost::asio;

int main(int argc, char** argv) {

    if (argc < 4)
    {
        std::cerr << "Usage: " << argv[0] << " exchange <agent_id> <port>" << "\n";
        std::cerr << "       " << argv[0] << " trader <agent_id> <port>" << "\n";
        return 1;
    }

    std::string agent_type { argv[1] };
    int agent_id { std::stoi(argv[2]) };
    unsigned int port { static_cast<unsigned int>(std::stoi(argv[3])) };

    asio::io_context io_context;
    if (agent_type == "exchange")
    {
        ExchangeAgent exchange{io_context, agent_id, "LSE", 9999};
        exchange.addTradeableAsset("AAPL");
        exchange.start();
    }
    else {
        ExampleTrader trader{io_context, agent_id, port};
        trader.connect("127.0.0.1:9999", "LSE", [&](){
            trader.subscribeToMarket("LSE", "AAPL");
            if (agent_id == 3) {
                trader.placeLimitOrder("LSE", Order::Side::BID, "AAPL", 100, 100.0);
            }
        });
        trader.start();
    }

    return 0;
}