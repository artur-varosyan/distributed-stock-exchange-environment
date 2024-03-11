#ifndef AGENT_HPP
#define AGENT_HPP

#include <iostream>
#include <tuple>
#include <boost/asio.hpp>
#include <boost/bimap.hpp>

#include "../networking/networkentity.hpp"
#include "../message/message.hpp"
#include "../message/messagetype.hpp"

namespace asio = boost::asio;

class Agent : public NetworkEntity
{
public:
    typedef boost::bimap<std::string, ipv4_address> address_book;

    Agent() = delete;
    virtual ~Agent() = default;

    Agent(asio::io_context& io_context, int agent_id, unsigned int port)
    : io_context_(io_context),
      agent_id{agent_id},
      port{port},
      NetworkEntity(io_context, port, port),
      known_agents{}
    {
    }

    Agent(asio::io_context& io_context, int agent_id)
    : io_context_(io_context),
      agent_id{agent_id},
      port{kUdpPort},
      NetworkEntity(io_context, kTcpPort, kUdpPort),
      known_agents{}
    {
    }

    /** Starts listening for incoming messages and broadcasts. */
    void start();

    /** Establishes a lasting connection with the agent at the given address. */
    void connect(ipv4_view address, std::string_view agent_name, std::function<void()> const& callback);

    /** Sends a message to the known agent with the given name */
    void sendMessageTo(std::string_view agent_name, MessagePtr message);

    /** Sends a broadcast to the agent with the given name */
    void sendBroadcastTo(std::string_view agent_name, MessagePtr message);

    /** Adds the given agent to the address book. */
    void addToAddressBook(ipv4_view address, std::string_view agent_name);

    /** Removes the given agent from the address book. */
    void removeFromAddressBook(std::string_view agent_name);


    /** Derived classes must implement these: */

    /** Handles an incoming message and returns the message to send. */
    virtual std::optional<MessagePtr> handleMessageFrom(std::string_view sender, MessagePtr message) = 0;

    /** Handles an incoming broadcast. */
    virtual void handleBroadcastFrom(std::string_view agent_name, MessagePtr message) = 0;

protected:

    /** The unique ID of the agent in the simulation. */
    int agent_id;

    /** A bidirectional map of known agent names and addresses. */
    address_book known_agents;

    /** The port the agent is listening on. */
    unsigned int port;

private:

    /** On receiving a new message, identifies the sender, adding to the address book if needed. */
    std::optional<MessagePtr> handleMessage(ipv4_view sender, MessagePtr message) override;

    /** On receiving a new broadcast, identifies the sender, adding to the address book if needed. */
    void handleBroadcast(ipv4_view sender, MessagePtr message) override;

    static constexpr unsigned int kTcpPort = 8080;
    static constexpr unsigned int kUdpPort = 8080;

    asio::io_context& io_context_;
};

#endif