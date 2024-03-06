#include <iostream>
#include <boost/asio.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/export.hpp>

#include "agent/echoagent.hpp"
#include "message/message.hpp"
#include "message/messagetype.hpp"

namespace asio = boost::asio;

int main(int argc, char** argv) {

    // if (argc < 2)
    // {
    //     std::cerr << "Usage: " << argv[0] << " <port>" << "\n";
    //     return 1;
    // }

    Message msg{MessageType::INIT};
    std::stringstream ss;
    boost::archive::text_oarchive oa{ss};
    oa << msg;
    std::cout << ss.str() << "\n";

    asio::io_context io_context;
    EchoAgent agent{io_context};
    agent.start();

    return 0;
}