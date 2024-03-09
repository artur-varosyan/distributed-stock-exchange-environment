#include <iostream>
#include <boost/asio.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/shared_ptr.hpp>

#include "agent/traderagent.hpp"
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
    TraderAgent agent{io_context, agent_id, port};

    std::shared_ptr<MarketDataMessage> msg = std::make_shared<MarketDataMessage>();

    msg->sender_id = agent_id;
    msg->ticker = "AAPL";
    msg->price = 100.0;
    msg->timestamp = 123456789;

    std::shared_ptr<Message> msgptr = std::dynamic_pointer_cast<Message>(msg);

    // Serialise the message
    std::stringstream ss;
    boost::archive::text_oarchive oa{ss};
    oa << msgptr;
    std::cout << "Serialised message:" << std::endl;
    std::cout << ss.str() << std::endl;

    agent.start();
    std::cout << "Agent started listening..." << "\n";

    return 0;
}