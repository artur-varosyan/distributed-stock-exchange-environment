#ifndef CONFIG_READER_HPP
#define CONFIG_READER_HPP

#include <string>
#include "../pugi/pugixml.hpp"

#include "simulationconfig.hpp"
#include "agentconfig.hpp"
#include "traderconfig.hpp"
#include "exchangeconfig.hpp"

/** Utility to read the simulation configuration files. */
class ConfigReader
{
public:

    ConfigReader() = delete;
    
    /** Reads the given XML configuration file and returns the list of agent configs. */
    static SimulationConfigPtr readConfig(std::string& filepath);

    /** Configures the agent based on the XML tag. */
    static AgentConfigPtr configureAgent(int id, pugi::xml_node& xml_node, std::string& addr, std::unordered_map<std::string, std::string>& exchange_addr);

private:

    static ExchangeConfigPtr configureExchange(int id, pugi::xml_node& xml_node, std::string& addr);

    static AgentConfigPtr configureTrader(int id, pugi::xml_node& xml_node, std::string& addr, std::unordered_map<std::string, std::string>& exchange_addr, AgentType trader_type);

    static AgentConfigPtr configureArbitrageur(int id, pugi::xml_node& xml_node, std::string& addr, std::unordered_map<std::string, std::string>& exchange_addr);

    static AgentConfigPtr configureMarketWatcher(int id, pugi::xml_node& xml_node, std::string& addr, std::unordered_map<std::string, std::string>& exchange_addr);

    static AgentConfigPtr configureTraderZIP(int id, pugi::xml_node& xml_node, std::string& addr, std::unordered_map<std::string, std::string>& exchange_addrs);
};

#endif