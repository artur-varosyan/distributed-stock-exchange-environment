#include "configreader.hpp"
#include "../agent/agentfactory.hpp"

SimulationConfigPtr ConfigReader::readConfig(std::string& filepath)
{
    pugi::xml_document doc;
    pugi::xml_parse_result res = doc.load_file(filepath.c_str());
    if (res.status != pugi::xml_parse_status::status_ok)
    {
        throw std::runtime_error("Failed to load configuration file: " + filepath);
    }

    pugi::xml_node simulation = doc.child("simulation");

    // Parse through the available instances
    std::vector<std::string> exchange_addrs;
    std::vector<std::string> trader_addrs;
    
    pugi::xml_node instances = simulation.child("instances");
    for (auto instance : instances.children())
    {
        std::string ip {instance.attribute("ip").value()};
        std::string port {instance.attribute("port").value()};
        std::string addr { ip + std::string{":"} + port };

        if (std::string{instance.attribute("agent-type").value()} == std::string{"exchange"})
        {
            exchange_addrs.push_back(addr);
        }
        else
        {
            trader_addrs.push_back(addr);
        }
    }

    // Parse through the configured agents
    int agent_id = 0;
    pugi::xml_node agents = simulation.child("agents");

    // Exchanges
    std::unordered_map<std::string, std::string> exchange_addrs_map; 
    std::vector<ExchangeConfigPtr> exchange_configs;

    int instance_id = 0;
    pugi::xml_node exchanges = agents.child("exchanges");
    for (auto exchange : exchanges.children())
    {
        ExchangeConfigPtr exchange_config = configureExchange(agent_id, exchange, exchange_addrs.at(instance_id));
        exchange_addrs_map.insert({exchange_config->name, exchange_addrs.at(instance_id)});
        exchange_configs.push_back(exchange_config);
        ++instance_id;
        ++agent_id;
    }

    // Traders
    std::vector<AgentConfigPtr> trader_configs;

    instance_id = 0;
    pugi::xml_node traders = agents.child("traders");
    for (auto trader : traders.children())
    {
        AgentConfigPtr trader_config = configureTrader(agent_id, trader, trader_addrs.at(instance_id), exchange_addrs_map);
        trader_configs.push_back(trader_config);
        ++instance_id;
        ++agent_id;
    }

    SimulationConfigPtr simulation_config = std::make_shared<SimulationConfig>(exchange_configs, trader_configs);
    return simulation_config;
}

ExchangeConfigPtr ConfigReader::configureExchange(int id, pugi::xml_node& xml_node, std::string& addr)
{
    ExchangeConfigPtr exchange_config = std::make_shared<ExchangeConfig>();

    exchange_config->agent_id = id;
    
    std::string type_tag { xml_node.name() };
    exchange_config->type = AgentFactory::getAgentTypeForTag(type_tag);

    exchange_config->addr = addr;
    exchange_config->name = std::string{xml_node.attribute("name").value()};
    exchange_config->tickers = std::vector{std::string{xml_node.attribute("ticker").value()}};
    exchange_config->connect_time = std::atoi(xml_node.attribute("connect-time").value());
    exchange_config->trading_time = std::atoi(xml_node.attribute("trading-time").value());

    return exchange_config;
}

AgentConfigPtr ConfigReader::configureTrader(int id, pugi::xml_node& xml_node, std::string& addr, std::unordered_map<std::string, std::string>& exchange_addrs)
{
    TraderConfigPtr trader_config = std::make_shared<TraderConfig>();
    trader_config->agent_id = id;

    std::string type_tag { xml_node.name() };
    trader_config->type = AgentFactory::getAgentTypeForTag(type_tag);

    trader_config->addr = addr;
    trader_config->exchange_name = std::string{xml_node.attribute("exchange").value()};
    trader_config->exchange_addr = exchange_addrs.at(trader_config->exchange_name);
    trader_config->limit = std::atoi(xml_node.attribute("limit").value());
    trader_config->delay = std::atoi(xml_node.attribute("delay").value());
    trader_config->ticker = std::string{xml_node.attribute("ticker").value()};
    
    std::string side {xml_node.attribute("side").value()};
    if (side == "buy") trader_config->side = Order::Side::BID;
    else if (side == "sell") trader_config->side = Order::Side::ASK;

    return std::static_pointer_cast<AgentConfig>(trader_config);
}

