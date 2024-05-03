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

    // Parse through the general configuration
    pugi::xml_node general = simulation.child("general");
    int time = general.child("time").text().as_int();
    int repetitions = general.child("repetitions").text().as_int();

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

    // Other agents
    std::vector<AgentConfigPtr> trader_configs;

    instance_id = 0;
    pugi::xml_node traders = agents.child("traders");
    for (auto trader : traders.children())
    {
        AgentConfigPtr agent_config = configureAgent(agent_id, trader, trader_addrs.at(instance_id), exchange_addrs_map);
         
        trader_configs.push_back(agent_config);
        ++instance_id;
        ++agent_id;
    }

    SimulationConfigPtr simulation_config = std::make_shared<SimulationConfig>(repetitions, time, exchange_configs, trader_configs);
    return simulation_config;
}

AgentConfigPtr ConfigReader::configureAgent(int id, pugi::xml_node& xml_node, std::string& addr, std::unordered_map<std::string, std::string>& exchange_addrs)
{
    std::string type_tag { xml_node.name() };
    AgentType type = AgentFactory::getAgentTypeForTag(type_tag);

    switch (type)
    {
        case AgentType::TRADER_ZIC:
        {
            return configureTrader(id, xml_node, addr, exchange_addrs, type);
        }
        case AgentType::TRADER_ZIP:
        {
            return configureTraderZIP(id, xml_node, addr, exchange_addrs);
        }
        case AgentType::TRADER_SHVR:
        {
            return configureTrader(id, xml_node, addr, exchange_addrs, type);
        }
        case AgentType::ARBITRAGE_TRADER:
        {
            return configureArbitrageur(id, xml_node, addr, exchange_addrs);
        }
        case AgentType::MARKET_WATCHER:
        {
            return configureMarketWatcher(id, xml_node, addr, exchange_addrs);
        }
        default:
        {
            throw std::runtime_error("Unknown XML tag in configuration file");
        }
    }
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

AgentConfigPtr ConfigReader::configureTrader(int id, pugi::xml_node& xml_node, std::string& addr, std::unordered_map<std::string, std::string>& exchange_addrs, AgentType trader_type)
{
    TraderConfigPtr trader_config = std::make_shared<TraderConfig>();
    trader_config->agent_id = id;

    trader_config->type = trader_type;

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

AgentConfigPtr ConfigReader::configureArbitrageur(int id, pugi::xml_node& xml_node, std::string& addr, std::unordered_map<std::string, std::string>& exchange_addrs)
{
    ArbitrageurConfigPtr config = std::make_shared<ArbitrageurConfig>();
    config->agent_id = id;
    config->addr = addr;

    std::string type_tag { xml_node.name() };
    config->type = AgentFactory::getAgentTypeForTag(type_tag);

    config->exchange0_name = std::string{xml_node.attribute("exchange0").value()};
    config->exchange0_addr = exchange_addrs.at(config->exchange0_name);

    config->exchange1_name = std::string{xml_node.attribute("exchange1").value()};
    config->exchange1_addr = exchange_addrs.at(config->exchange1_name);
    
    config->ticker = std::string{xml_node.attribute("ticker").value()};
    config->alpha = std::stod(xml_node.attribute("alpha").value());
    config->delay = std::atoi(xml_node.attribute("delay").value());

    return std::static_pointer_cast<AgentConfig>(config);
}

AgentConfigPtr ConfigReader::configureMarketWatcher(int id, pugi::xml_node& xml_node, std::string& addr, std::unordered_map<std::string, std::string>& exchange_addrs)
{
    MarketWatcherConfigPtr config = std::make_shared<MarketWatcherConfig>();
    config->agent_id = id;
    config->addr = addr;
    config->type = AgentType::MARKET_WATCHER;

    config->exchange_name = std::string{xml_node.attribute("exchange").value()};
    config->exchange_addr = exchange_addrs.at(config->exchange_name);
    
    config->ticker = std::string{xml_node.attribute("ticker").value()};

    return std::static_pointer_cast<AgentConfig>(config);
}

AgentConfigPtr ConfigReader::configureTraderZIP(int id, pugi::xml_node& xml_node, std::string& addr, std::unordered_map<std::string, std::string>& exchange_addrs)
{
    ZIPConfigPtr config = std::make_shared<ZIPConfig>();
    config->agent_id = id;

    config->type = AgentType::TRADER_ZIP;

    config->addr = addr;
    config->exchange_name = std::string{xml_node.attribute("exchange").value()};
    config->exchange_addr = exchange_addrs.at(config->exchange_name);
    config->limit = std::atoi(xml_node.attribute("limit").value());
    config->delay = std::atoi(xml_node.attribute("delay").value());
    config->ticker = std::string{xml_node.attribute("ticker").value()};
    
    std::string side {xml_node.attribute("side").value()};
    if (side == "buy") config->side = Order::Side::BID;
    else if (side == "sell") config->side = Order::Side::ASK;

    config->min_margin = std::stod(xml_node.attribute("min-margin").value());
    config->trade_interval = std::stoull(xml_node.attribute("trade-interval").value());
    config->liquidity_interval = std::stoull(xml_node.attribute("liquidity-interval").value());


    return std::static_pointer_cast<AgentConfig>(config);

}

