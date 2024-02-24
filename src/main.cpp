#include <iostream>
#include <boost/asio.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/export.hpp>

#include "networking/networkentity.hpp"
#include "message/message.hpp"
BOOST_CLASS_EXPORT(Message);
#include "message/market_data_message.hpp"
BOOST_CLASS_EXPORT(MarketDataMessage);

namespace asio = boost::asio;

int main(int argc, char** argv) {

    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " <port>" << "\n";
        return 1;
    }

    unsigned short port = std::stoi(argv[1]);

    asio::io_context io_context;
    NetworkEntity entity{io_context, port, port};

    asio::co_spawn(io_context, entity.start(), asio::detached);
    io_context.run();

    return 0;
}