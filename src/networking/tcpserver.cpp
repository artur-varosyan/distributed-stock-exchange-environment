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
    std::string address { connection->socket().remote_endpoint().address().to_string() };
    unsigned int port { connection->socket().remote_endpoint().port() };

    try {
        std::string read_buffer;
        while (true)
        {
            std::string message = co_await connection->read(read_buffer);

            std::string response = handleMessage(address, port, message);
            if (!response.empty())
            {
                co_await connection->send(response, false);
            }
        }
    }
    catch (std::exception& e)
    {
        // std::cout << "TCP message listener failed" << "\n";
        std::cout << "Connection with " << address << ":" << port << " dropped.\n";
        // std::cout << "Reason:\n" << e.what() << "\n";

        removeConnection(address, port);
    }
}

asio::awaitable<void> TCPServer::messageWriter(TCPConnectionPtr connection)
{
    std::string address { connection->socket().remote_endpoint().address().to_string() };
    unsigned int port { connection->socket().remote_endpoint().port() };
    
    try {
        while (connection->open())
        {
            // Wait for new message added to queue
            if (connection->queue().empty())
            {
                /** TODO: Decide how to handle this error code. */
                boost::system::error_code ec;
                co_await connection->timer().async_wait(asio::redirect_error(asio::use_awaitable, ec));
            }
            // Send message to the connection
            else
            {
                co_await asio::async_write(connection->socket(), asio::buffer(connection->queue().front()), asio::use_awaitable);
                connection->queue().pop();
            }
        }
    }
    catch (std::exception& e)
    {
        // std::cout << "TCP message writer failed" << "\n";
        std::cout << "Connection with " << address << ":" << port << " dropped.\n";
        // std::cout << "Reason:\n" << e.what() << "\n";

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

    // Start a writing coroutine to send messages to this connection
    asio::co_spawn(io_context_, messageWriter(connection), asio::detached);

    co_return;
}

asio::awaitable<void> TCPServer::listener()
{
    auto executor = co_await asio::this_coro::executor;
    tcp::acceptor acceptor(executor, {tcp::v4(), tcp_port_});

    while (true)
    {
        tcp::socket socket = co_await acceptor.async_accept(asio::use_awaitable);
        asio::co_spawn(executor, handleAccept(std::move(socket)), asio::detached);
    }
}

asio::awaitable<void> TCPServer::sendMessage(TCPConnectionPtr connection, std::string message, bool async)
{
    co_await connection->send(message, async);
}

asio::awaitable<void> TCPServer::connect(std::string address, const unsigned int port, std::function<void()> callback)
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

        // Make a callback to the caller
        callback();
        
        // Start listening for messages from this connection
        asio::co_spawn(io_context_, messageListener(connection), asio::detached);

        // Start a writing coroutine to send messages to this connection
        asio::co_spawn(io_context_, messageWriter(connection), asio::detached);
    }
    catch (std::exception& e)
    {
        std::cout << "Exception: " << e.what() << std::endl;
        std::cout << "Failed to connect to " << address << ":" << port << std::endl;
        throw e;
    }
    
}