#include <iostream>
#include <boost/asio.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/export.hpp>

#include "agent/agent.hpp"

namespace asio = boost::asio;

int main(int argc, char** argv) {

    // if (argc < 2)
    // {
    //     std::cerr << "Usage: " << argv[0] << " <port>" << "\n";
    //     return 1;
    // }

    asio::io_context io_context;
    Agent agent{io_context};
    agent.start();

    return 0;
}