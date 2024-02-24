#include <iostream>
#include <boost/asio.hpp>

#include "tcpconnection.hpp"

namespace asio = boost::asio;

asio::awaitable<void> TCPConnection::send(std::string_view message)
{
    try {
        co_await asio::async_write(socket_, asio::buffer(message), asio::use_awaitable);
    }
    catch (std::exception& e)
    {
        std::cout << "Failed to send message. Exception " << e.what() << "\n";
    }
}

asio::awaitable<std::string> TCPConnection::read()
{
    char data[1024];

    std::size_t n = co_await socket_.async_read_some(asio::buffer(data), asio::use_awaitable);
    std::cout << "Message from " << socket_.remote_endpoint() << ": " << std::string(data, n) << " of size " << n << "\n";

    co_return std::string(data, n);
}