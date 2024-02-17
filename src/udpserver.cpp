#include <boost/asio.hpp>
#include <iostream>

#include "udpserver.hpp"

namespace asio = boost::asio;
using asio::ip::udp;

void UDPServer::start()
{
    std::cout << "Starting a UDP server on port " << udp_port_ << "\n";

    asio::co_spawn(io_context_, listener(), asio::detached);
    io_context_.run();
}

asio::awaitable<void> UDPServer::listener()
{
    auto executor = co_await asio::this_coro::executor;
    std::cout << "Listening for UDP on port " << udp_port_ << "\n";

    char data[1024];

    while (true)
    {
        udp::endpoint endpoint;
        std::size_t n = co_await socket_.async_receive_from(asio::buffer(data), endpoint, asio::use_awaitable);
        
        std::string message(data, n);
        handleBroadcast(message);
    }
}