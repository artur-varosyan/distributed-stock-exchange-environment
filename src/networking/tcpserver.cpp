#include "tcpserver.hpp"
#include <boost/asio.hpp>

namespace asio = boost::asio;
using asio::ip::tcp;

typedef std::shared_ptr<TCPConnection> TCPConnectionPtr;

asio::awaitable<void> TCPServer::start()
{
    asio::signal_set signals(io_context_, SIGINT, SIGTERM);
    signals.async_wait([&](auto, auto){ io_context_.stop(); });

    co_await listener();
}

asio::awaitable<void> TCPServer::messageListener(TCPConnectionPtr connection)
{
    std::string address { connection->getSocket().remote_endpoint().address().to_string() };
    unsigned int port { connection->getSocket().remote_endpoint().port() };

    try {
        while (true)
        {
            std::string message = co_await connection->read();

            std::string response = handleMessage(address, port, message);
            if (!response.empty())
            {
                co_await connection->send(response);
            }
        }
    }
    catch (std::exception& e)
    {
        std::cout << "Exception in message listener: " << e.what() << "\n";

        // Remove connection from list
        removeConnection(address, port);
    }
}

asio::awaitable<void> TCPServer::handleAccept(tcp::socket socket)
{
    std::cout << "Accepted connection from " << socket.remote_endpoint() << "\n";

    std::string address { socket.remote_endpoint().address().to_string() };
    unsigned int port { socket.remote_endpoint().port() };
    
    // Create a shared pointer to this connection and add to list
    TCPConnectionPtr connection = std::make_shared<TCPConnection>(std::move(socket));
    addConnection(address, port, connection);

    // Start listening for messages from this connection
    asio::co_spawn(io_context_, messageListener(connection), asio::detached);

    co_return;
}

asio::awaitable<void> TCPServer::listener()
{
    auto executor = co_await asio::this_coro::executor;
    tcp::acceptor acceptor(executor, {tcp::v4(), tcp_port_});
    std::cout << "Listening for TCP on port " << tcp_port_ << "\n";

    while (true)
    {
        tcp::socket socket = co_await acceptor.async_accept(asio::use_awaitable);
        asio::co_spawn(executor, handleAccept(std::move(socket)), asio::detached);
    }
}

asio::awaitable<void> TCPServer::sendMessage(TCPConnectionPtr connection, std::string_view message)
{
    co_await connection->send(message);
}

asio::awaitable<TCPConnectionPtr> TCPServer::connect(std::string address, const unsigned int port) 
{
    asio::ip::address addr = asio::ip::make_address(address);
    tcp::endpoint endpoint(addr, port);
    tcp::socket socket(io_context_);

    try
    {
        co_await socket.async_connect(endpoint, asio::use_awaitable);

        // Create a shared pointer to this connection and add to list
        TCPConnectionPtr connection = std::make_shared<TCPConnection>(std::move(socket));
        addConnection(address, port, connection);
        
        // Start listening for messages from this connection
        asio::co_spawn(io_context_, messageListener(connection), asio::detached);

        co_return connection;
    }
    catch (std::exception& e)
    {
        std::cout << "Failed to connect to " << socket.remote_endpoint() << "\n";
        std::cout << "Exception: " << e.what() << "\n";
        throw e;
    }
    
}