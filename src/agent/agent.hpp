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

class Agent : protected NetworkEntity
{
public:
    typedef boost::bimap<std::string, ipv4_address> address_book;

    Agent() = delete;
     virtual ~Agent() = default;

    Agent(asio::io_context& io_context)
    : io_context_(io_context),
      NetworkEntity(io_context, kTcpPort, kUdpPort),
      known_agents{}
    {
    }

    /** Starts listening for incoming messages and broadcasts. */
    void start();

    /** Establishes a lasting connection with the agent at the given address. */
    bool connect(ipv4_view address, std::string_view agent_name);

    /** Adds the given agent to the address book. */
    void addToAddressBook(ipv4_view address, std::string_view agent_name);

    /** Removes the given agent from the address book. */
    void removeFromAddressBook(std::string_view agent_name);


    /** Derived classes must implement these: */

    /** Handles an incoming message and returns the message to send. */
    virtual std::optional<Message> handleMessageFrom(std::string_view sender, Message message) = 0;

    /** Handles an incoming broadcast. */
    virtual void handleBroadcastFrom(std::string_view agent_name, Message message) = 0;

protected:

    /** A bidirectional map of known agent names and addresses. */
    address_book known_agents;

private:

    /** On receiving a new message, identifies the sender, adding to the address book if needed. */
    std::optional<Message> handleMessage(ipv4_view sender, Message message) override;

    /** On receiving a new broadcast, identifies the sender, adding to the address book if needed. */
    void handleBroadcast(ipv4_view sender, Message message) override;

    static constexpr unsigned int kTcpPort = 8080;
    static constexpr unsigned int kUdpPort = 8080;

    asio::io_context& io_context_;
};

#endif