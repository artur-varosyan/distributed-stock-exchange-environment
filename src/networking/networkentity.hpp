#ifndef NETWORK_ENTITY_HPP
#define NETWORK_ENTITY_HPP

#include <iostream>
#include <boost/asio.hpp>
#include <boost/bimap.hpp>

#include "tcpserver.hpp"
#include "udpserver.hpp"
#include "../message/message.hpp"

namespace asio = boost::asio;
using asio::ip::tcp;
using asio::ip::udp;

typedef std::shared_ptr<Message> MessagePtr;

class NetworkEntity : protected TCPServer, protected UDPServer
{
public:
    typedef std::shared_ptr<TCPConnection> TCPConnectionPtr;
    typedef std::string ipv4_address;
    typedef std::string_view ipv4_view;
    typedef boost::bimap<ipv4_address, TCPConnectionPtr> bimap;

    NetworkEntity() = delete;
    virtual ~NetworkEntity() = default;

    NetworkEntity(asio::io_context& io_context, unsigned short tcp_port, unsigned short udp_port)
    : io_context_(io_context),
      TCPServer(io_context, tcp_port),
      UDPServer(io_context, udp_port),
      connections_{}
    {
    }

    /** Starts both servers sharing the same context */
    void start();

    /** Establishes a lasting TCP connection with the given IPv4 address. */
    void connect(ipv4_view address);

    /** Sends a broadcast to the given IPv4 address. */
    void sendBroadcast(ipv4_view address, MessagePtr message);

    /** Sends a message to the given IPv4 address. */
    void sendMessage(ipv4_view address, MessagePtr message);

    /** Combines IP address with port into a single string. */
    std::string concatAddress(std::string_view address, unsigned int port);

    /** Splits a combined address into IP address and port. */
    std::pair<std::string, unsigned int> splitAddress(ipv4_view address);


    /** Derived classes must implement these methods: */

    /** Handles an incoming message and returns the message to send. All derived classes must implement this. */
    virtual std::optional<MessagePtr> handleMessage(ipv4_view sender, MessagePtr message) = 0;

    /** Handles an incoming broadcast. All derived classes must implement this. */
    virtual void handleBroadcast(ipv4_view sender, MessagePtr message) = 0;

private:

    /** Adds a given connection to the bimap of open connections. */
    void addConnection(std::string_view address, unsigned int port, TCPConnectionPtr connection) override;

    /** Removes a given connection from the bimap of open connections. */
    void removeConnection(std::string_view address, unsigned int port) override;

    /** Handles an incoming TCP message from one of the existing connections. */
    std::string handleMessage(std::string_view sender_adress, unsigned int sender_port, std::string_view message) override;

    /** Handles an incoming UDP broadcast. */
    void handleBroadcast(std::string_view sender_adress, unsigned int sender_port, std::string_view message) override;

    /** Serialises a message into a string to be sent. */
    std::string serialiseMessage(MessagePtr message);

    /** Deserialises incoming strings into messages. */
    MessagePtr deserialiseMessage(std::string_view message);

    /** The bidirectional map of currently open TCP connections. */
    bimap connections_;

    asio::io_context& io_context_;
};


#endif