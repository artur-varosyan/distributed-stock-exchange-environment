#include <iostream>
#include <boost/asio.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/shared_ptr.hpp>

#include <chrono>
#include <thread>

#include "agent/exampletrader.hpp"
#include "message/message.hpp"
#include "message/messagetype.hpp"
#include "message/market_data_message.hpp"

namespace asio = boost::asio;

int main(int argc, char** argv) {

    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " <agent_id> <port>" << "\n";
        return 1;
    }

    int agent_id { std::stoi(argv[1]) };
    unsigned int port { static_cast<unsigned int>(std::stoi(argv[2])) };

    asio::io_context io_context;
    ExampleTrader trader{io_context, agent_id, port};

    if (agent_id == 0) {
        trader.start();
        
    } else {
        std::shared_ptr<MarketDataMessage> msg = std::make_shared<MarketDataMessage>();

        msg->sender_id = agent_id;
        msg->ticker = "AAPL";
        msg->price = 100.0;
        msg->timestamp = 123456789;

        trader.connect("127.0.0.1:8080", "trader0", [&](){
            trader.sendMessage("127.0.0.1:8080", std::dynamic_pointer_cast<Message>(msg));
        });
        
        trader.start();
    }

    return 0;
}