#ifndef NETWORK_ENTITY_HPP
#define NETWORK_ENTITY_HPP

#include <iostream>
#include <memory>
#include <functional>
#include <optional>
#include <boost/asio.hpp>
#include <boost/bimap.hpp>

#include "tcpserver.hpp"
#include "udpserver.hpp"
#include "../message/message.hpp"
#include "../message/config_message.hpp"

class Agent;

namespace asio = boost::asio;
using asio::ip::tcp;
using asio::ip::udp;

class NetworkEntity : protected TCPServer, protected UDPServer
{
public:
    typedef std::shared_ptr<TCPConnection> TCPConnectionPtr;
    typedef std::string ipv4_address;
    typedef std::string_view ipv4_view;
    typedef boost::bimap<ipv4_address, TCPConnectionPtr> bimap;

    NetworkEntity() = delete;
    virtual ~NetworkEntity() = default;

    NetworkEntity(asio::io_context& io_context, unsigned short port)
    : io_context_(io_context),
      port_(port),
      addr_(std::nullopt),
      agent_(std::nullopt),
      TCPServer(io_context, port),
      UDPServer(io_context, port),
      connections_{}
    {
    }

    NetworkEntity(asio::io_context& io_context, std::string addr, unsigned short port)
    : io_context_(io_context),
      port_(port),
      addr_(addr),
      agent_(std::nullopt),
      TCPServer(io_context, port),
      UDPServer(io_context, port),
      connections_{}
    {
    }

    /** Starts both servers and listens for incoming connections. */
    virtual void start();

    /** Establishes a lasting TCP connection with the given IPv4 address. */
    void connect(ipv4_view address, std::function<void()> const& callback);

    /** Sends a broadcast to the given IPv4 address. */
    void sendBroadcast(ipv4_view address, MessagePtr message);

    /** Sends a message to the given IPv4 address. */
    void sendMessage(ipv4_view address, MessagePtr message, bool async);

    /** Returns the listening port of the NetworkEntity. */
    unsigned int port();

    /** Returns the address of the NetworkEntity. Must be called after address is known. */
    std::string addr();

    /** Sets the agent running inside this NetworkEntity. */
    void setAgent(std::shared_ptr<Agent> agent);

private:

    /** Returns the agent running inside this NetworkEntity. */
    std::shared_ptr<Agent> agent();

    /** Initialises the agent running inside this NetworkEntity using a config message. */
    void configureEntity(std::string_view sender_address, ConfigMessagePtr msg);

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

    /** Combines IP address with port into a single string. */
    std::string concatAddress(std::string_view address, unsigned int port);

    /** Splits a combined address into IP address and port. */
    std::pair<std::string, unsigned int> splitAddress(ipv4_view address);

    /** The IO context used for networking. */
    asio::io_context& io_context_;

    /** The bidirectional map of currently open TCP connections. */
    bimap connections_;

    /** The port of this NetworkEntity. */
    unsigned int port_;

    /** The IPv4 address of this NetworkEntity. */
    std::optional<std::string> addr_;

    /** Pointer to an Agent. May be empty before agent is initialised. */
    std::optional<std::shared_ptr<Agent>> agent_;
};


#endif