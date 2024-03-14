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
        StockExchange exchange{io_context, agent_id, "LSE", 9999};
        exchange.addTradeableAsset("AAPL");
        exchange.start();
    }
    else {
        ExampleTrader trader{io_context, agent_id, port};
        
        std::mutex m;
        std::condition_variable cv;
        std::thread t([&](){
            std::unique_lock<std::mutex> lk(m);
            // Wait until connection with exchange established
            cv.wait(lk);

            for (int i=0; i < 1500; i++) {
                if (agent_id == 2) {
                    trader.placeLimitOrder("LSE", Order::Side::BID, "AAPL", 100, 100.0);
                    std::this_thread::sleep_for(std::chrono::milliseconds(1500));
                }
                else if (agent_id == 3) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                    trader.placeLimitOrder("LSE", Order::Side::ASK, "AAPL", 100, 100.0);
                }
            }
        });

        trader.connect("127.0.0.1:9999", "LSE", [&](){
            trader.subscribeToMarket("LSE", "AAPL");
            cv.notify_one();
        });
        trader.start();
    }

    return 0;
}