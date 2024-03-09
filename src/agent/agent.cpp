#include <iostream>
#include <string>

#include "agent.hpp"

void Agent::start()
{
    NetworkEntity::start();
}

void Agent::connect(ipv4_view address, std::string_view agent_name, std::function<void()> const& callback)
{
    NetworkEntity::connect(address, [=, this]() {
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
    // Check if sender is in known agents address book
    if (known_agents.right.find(std::string(sender)) != known_agents.right.end())
    {
        // Known sender from address book
        return handleMessageFrom(known_agents.right.at(std::string(sender)), message);
    }
    else
    {
        // Unknown sender
        return handleMessageFrom("unknown", message);
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
    if (known_agents.left.find(std::string{agent_name}) != known_agents.left.end())
    {
        std::cout << "Agent name exists in address book" << "\n";
        NetworkEntity::sendMessage(known_agents.left.at(std::string{agent_name}), message);
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
        NetworkEntity::sendBroadcast(known_agents.left.at(std::string{agent_name}), message);
    }
    else
    {
        throw std::runtime_error("Unknown agent name");
    }
}