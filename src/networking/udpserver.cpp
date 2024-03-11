#include <boost/asio.hpp>
#include <iostream>

#include "udpserver.hpp"

namespace asio = boost::asio;
using asio::ip::udp;

asio::awaitable<void> UDPServer::start()
{
    // std::cout << "Starting a UDP server on port " << udp_port_ << "\n";

    co_await listener();
}

asio::awaitable<void> UDPServer::listener()
{
    auto executor = co_await asio::this_coro::executor;
    // std::cout << "Listening for UDP on port " << udp_port_ << "\n";

    char data[1024];

    while (true)
    {
        udp::endpoint endpoint;
        std::size_t n = co_await socket_.async_receive_from(asio::buffer(data), endpoint, asio::use_awaitable);
        
        std::string message(data, n);
        handleBroadcast(endpoint.address().to_string(), endpoint.port(), message);
    }
}

asio::awaitable<void> UDPServer::sendBroadcast(std::string_view address, const unsigned int port, std::string_view message)
{
    udp::endpoint endpoint(asio::ip::make_address(address), port);
    co_await sendBroadcast(endpoint, message);
}

asio::awaitable<void> UDPServer::sendBroadcast(udp::endpoint endpoint, std::string_view message)
{
    co_await socket_.async_send_to(asio::buffer(message), endpoint, asio::use_awaitable);
}