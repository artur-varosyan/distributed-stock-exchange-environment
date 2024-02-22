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

    MarketDataMessage msgOg;
    msgOg.sender_id = 1;
    msgOg.receiver_ids = {2, 3, 4};
    msgOg.symbol = "AAPL";
    msgOg.price = 123.45;

    Message* msg = &msgOg;

    // Serialize the message
    std::ostringstream oss;
    boost::archive::text_oarchive oa{oss};
    oa << *msg;

    std::string serialized_message = oss.str();
    std::cout << serialized_message << "\n";

    // Deserialize the message
    std::istringstream iss{serialized_message};
    boost::archive::text_iarchive ia{iss};
    std::cout << iss.str() << "\n";

    Message deserialized_message;
    ia >> deserialized_message;

    if (deserialized_message.type == Message::Type::MARKET_DATA) {
        std::cout << "Deserialized message is of type MARKET_DATA\n";

        MarketDataMessage* mdm = dynamic_cast<MarketDataMessage*>(&deserialized_message);

    }


    asio::io_context io_context;
    NetworkEntity entity{io_context, port, port};

    asio::co_spawn(io_context, entity.start(), asio::detached);
    io_context.run();

    return 0;
}