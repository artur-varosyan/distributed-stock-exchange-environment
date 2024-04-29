#ifndef AGENT_HPP
#define AGENT_HPP

#include <iostream>
#include <tuple>
#include <boost/asio.hpp>
#include <boost/bimap.hpp>

#include "../config/agentconfig.hpp"
#include "../message/message.hpp"
#include "../message/messagetype.hpp"

namespace asio = boost::asio;

// Forward declaration to avoid circular dependency
class NetworkEntity;

class Agent : std::enable_shared_from_this<Agent>
{
public:
    typedef std::string ipv4_address;
    typedef std::string_view ipv4_view;
    typedef boost::bimap<std::string, ipv4_address> address_book;

    Agent() = delete;
    virtual ~Agent() = default;

    Agent(NetworkEntity* network, AgentConfigPtr config)
    : agent_id{config->agent_id},
      network_{network}
    {
    }

    /** Starts listening for incoming messages and broadcasts. */
    virtual void start();

    /** Returns agent ID. */
    int getAgentId();

    /** Establishes a lasting connection with the agent at the given address. */
    void connect(ipv4_address address, std::string agent_name, std::function<void()> const& callback);

    /** Sends a message to the known agent with the given name. */
    void sendMessageTo(std::string_view agent_name, MessagePtr message, bool async = false);

    /** Sends a broadcast to the agent with the given name. */
    void sendBroadcastTo(std::string_view agent_name, MessagePtr message);

    /** Sends a broadcast to the agent at the given address. */
    void sendBroadcast(std::string_view address, MessagePtr message);

    /** Adds the given agent to the address book. */
    void addToAddressBook(ipv4_view address, std::string_view agent_name);

    /** Removes the given agent from the address book. */
    void removeFromAddressBook(std::string_view agent_name);

    /** On receiving a new message, identifies the sender, adding to the address book if needed. */
    std::optional<MessagePtr> handleMessage(ipv4_view sender, MessagePtr message);

    /** On receiving a new broadcast, identifies the sender, adding to the address book if needed. */
    void handleBroadcast(ipv4_view sender, MessagePtr message);

protected:

    /** Returns the port the agent is listening on. */
    unsigned int myPort();

    /** Returns the public IPv4 address of the agent. */
    std::string myAddr();

    /** Derived classes must implement these: */

    /** Handles an incoming message and returns the message to send. */
    virtual std::optional<MessagePtr> handleMessageFrom(std::string_view sender, MessagePtr message) = 0;

    /** Handles an incoming broadcast. */
    virtual void handleBroadcastFrom(std::string_view agent_name, MessagePtr message) = 0;

    /** The unique ID of the agent in the simulation. */
    int agent_id;

    /** A bidirectional map of known agent names and addresses. */
    address_book known_agents;

private:

    NetworkEntity* network();

    NetworkEntity* network_;
};

#endif