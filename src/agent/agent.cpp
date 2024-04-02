#include <iostream>
#include <string>

#include "agent.hpp"
#include "../networking/networkentity.hpp"

void Agent::start()
{

}

void Agent::connect(ipv4_view address, std::string_view agent_name, std::function<void()> const& callback)
{
    network()->connect(address, [=, this]() {
        addToAddressBook(address, agent_name);
        callback();
    });
}

void Agent::addToAddressBook(ipv4_view address, std::string_view agent_name)
{
    known_agents.left.insert({std::string{agent_name}, std::string{address}});
}

void Agent::removeFromAddressBook(std::string_view agent_name)
{
    known_agents.left.erase(std::string{agent_name});
}

std::optional<MessagePtr> Agent::handleMessage(ipv4_view sender, MessagePtr message)
{
    // std::cout << "In agent handle message" << "\n";

    // Check if sender is in known agents address book
    if (known_agents.right.find(std::string(sender)) != known_agents.right.end())
    {
        // Known sender from address book
        return handleMessageFrom(known_agents.right.at(std::string(sender)), message);
    }
    else
    {
        // Unknown sender, add to address book
        std::string agent_id = std::to_string(message->sender_id);
        addToAddressBook(sender, agent_id);
        return handleMessageFrom(agent_id, message);
    }
}

void Agent::handleBroadcast(ipv4_view sender, MessagePtr message)
{
    if (known_agents.right.find(std::string(sender)) != known_agents.right.end())
    {
        // Known sender from address book
        handleBroadcastFrom(known_agents.right.at(std::string(sender)), message);
    }
    else
    {
        // Unknown sender
        handleBroadcastFrom("unknown", message);
    }
}

void Agent::sendMessageTo(std::string_view agent_name, MessagePtr message)
{
    // std::cout << "Sending message to " << agent_name << "\n";
    if (known_agents.left.find(std::string{agent_name}) != known_agents.left.end())
    {
        // std::cout << "Agent found in address book\n";
        network()->sendMessage(known_agents.left.at(std::string{agent_name}), message);
    }
    else
    {
        throw std::runtime_error("Unknown agent name");
    }
}

void Agent::sendBroadcastTo(std::string_view agent_name, MessagePtr message)
{
    if (known_agents.left.find(std::string{agent_name}) != known_agents.left.end())
    {
        network()->sendBroadcast(known_agents.left.at(std::string{agent_name}), message);
    }
    else
    {
        throw std::runtime_error("Unknown agent name");
    }
}

void Agent::sendBroadcast(std::string_view address, MessagePtr message)
{
    network()->sendBroadcast(address, message);
}

NetworkEntity* Agent::network()
{
    if (!network_.has_value()) 
    {
        throw std::runtime_error("Agent has not been asigned to a network entity.");
    }

    return network_.value();
}

unsigned int Agent::myPort()
{
    return network()->port();
}

void Agent::setNetwork(NetworkEntity *network)
{
    network_ = network;
}