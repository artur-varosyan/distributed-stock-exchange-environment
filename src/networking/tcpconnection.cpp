#include <iostream>
#include <boost/asio.hpp>

#include "tcpconnection.hpp"

namespace asio = boost::asio;

asio::awaitable<void> TCPConnection::send(std::string_view message, bool async)
{
    try {
        if (async)
        {
            co_await asio::async_write(socket_, asio::buffer(message), asio::use_awaitable);
        }
        else 
        {
            asio::write(socket_, asio::buffer(message));
            co_return;
        }
    }
    catch (std::exception& e)
    {
        std::cout << "Failed to send message. Exception " << e.what() << "\n";
    }
}

asio::awaitable<std::string> TCPConnection::read(std::string& read_buffer)
{
    std::size_t n = co_await asio::async_read_until(socket_, asio::dynamic_buffer(read_buffer, 4096), "#END#", asio::use_awaitable);
    // std::cout << "Message from " << socket_.remote_endpoint() << ": " << std::string(data, n) << " of size " << n << "\n";

    std::string msg = read_buffer.substr(0, n);
    read_buffer.erase(0, n);
    co_return msg;
}