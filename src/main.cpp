#include <iostream>
#include <chrono>
#include <thread>

#include <boost/asio.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/program_options.hpp>

#include "networking/networkentity.hpp"

#include "agent/exampletrader.hpp"
#include "agent/stockexchange.hpp"
#include "agent/marketdatawatcher.hpp"
#include "agent/traderzic.hpp"
#include "agent/tradershvr.hpp"
#include "agent/orchestratoragent.hpp"

#include "message/message.hpp"
#include "message/messagetype.hpp"
#include "message/market_data_message.hpp"

#include "config/exchangeconfig.hpp"
#include "config/traderconfig.hpp"

namespace asio = boost::asio;
namespace po = boost::program_options;

std::string showUsage() 
{
    std::stringstream ss;
    ss << "Usage: " << "./simulation" << " <mode>" << "\n\n";
    ss << "Modes:\n";
    ss << "  " << "local" << "\t\t" << "run simulation in local mode" << "\n";
    ss << "  " << "orchestrator" << "\t" << "orchestrate the cloud simulation from this node" << "\n";
    ss << "  " << "cloud" << "\t\t" << "run a simulation node" << "\n";
    return ss.str();
}

std::string showLocalUsage() {
    std::stringstream ss;
    ss << "Usage: " << "./simulation local" << " <agent> <agent_id> [options]" << "\n\n";
    ss << "Agents:\n";
    ss << "  " << "exchange" << "\t" << "multithreaded stock exchange implementation" << "\n";
    ss << "  " << "watcher" << "\t" << "live market data watcher" << "\n";
    ss << "  " << "zic" << "\t\t" << "zero intelligence constrained trader" << "\n";
    ss << "  " << "shvr" << "\t\t" << "shaver trader" << "\n";
    ss << "\n";
    return ss.str();
}

void local_runner(int argc, char** argv)
{
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "show help message")
        ("port", po::value<unsigned short>()->default_value(8080), "set the port of the current agent")
        ("ticker", po::value<std::string>()->default_value(std::string{"AAPL"}), "set the ticker to trade")
        ("exchange-name", po::value<std::string>()->default_value(std::string{"LSE"}), "set the name of the exchange")
        ("connect-time", po::value<int>()->default_value(30), "(exchange only) the time allowed for traders to connect (seconds)")
        ("trading-time", po::value<int>()->default_value(60), "(exchange only) the time of the trading window (seconds)")
        ("delay", po::value<unsigned int>()->default_value(0), "(trader only) delayed start for trader (seconds)")
        ("side", po::value<std::string>()->default_value(std::string{"buyer"}), "(trader only) set the trader side: buyer or seller")
        ("limit", po::value<double>()->default_value(100), "(trader only) set the limit price of the trader")
        ("exchange-addr", po::value<std::string>()->default_value(std::string{"127.0.0.1:9999"}), "(trader only) set the IPv4 address of the exchange")
    ;

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (argc < 4 || vm.count("help"))
    {
        std::cout << "\n" << showLocalUsage() << desc << std::endl;
        exit(1);
    }

    std::string agent_type { argv[2] };
    int agent_id { std::stoi(argv[3]) };
    unsigned short port { vm["port"].as<unsigned short>() };

    asio::io_context io_context;
    NetworkEntity entity{io_context, std::string{"127.0.0.1"}, port};

    if (agent_type == "exchange")
    {
        // Create configuration
        ExchangeConfig config;
        config.agent_id = agent_id;
        config.name = vm["exchange-name"].as<std::string>();
        config.tickers = std::vector { vm["ticker"].as<std::string>() };
        config.connect_time = vm["connect-time"].as<int>();
        config.trading_time = vm["trading-time"].as<int>();

        std::shared_ptr<StockExchange> exchange (new StockExchange{&entity, &config});
        entity.setAgent(std::static_pointer_cast<Agent>(exchange));
        entity.start();
    }
    else if (agent_type == "trader") {

        // Create configuration
        TraderConfig config;
        config.agent_id = agent_id;
        config.exchange_name = vm["exchange-name"].as<std::string>();
        config.exchange_addr = vm["exchange-addr"].as<std::string>();
        config.ticker = vm["ticker"].as<std::string>();
        config.delay = vm["delay"].as<unsigned int>();

        std::shared_ptr<ExampleTrader> trader (new ExampleTrader{&entity, &config});
        entity.setAgent(std::static_pointer_cast<Agent>(trader));
        entity.start();
    }
    else if (agent_type == "watcher") {

        // Create configuration
        TraderConfig config;
        config.agent_id = agent_id;
        config.exchange_name = vm["exchange-name"].as<std::string>();
        config.exchange_addr = vm["exchange-addr"].as<std::string>();
        config.ticker = vm["ticker"].as<std::string>();
        config.delay = vm["delay"].as<unsigned int>();

        std::shared_ptr<MarketDataWatcher> watcher (new MarketDataWatcher{&entity, &config});
        entity.setAgent(std::static_pointer_cast<Agent>(watcher));
        entity.start();
    } 
    else if (agent_type == "zic") 
    {
        // Create configuration
        TraderConfig config;
        config.agent_id = agent_id;
        config.exchange_name = vm["exchange-name"].as<std::string>();
        config.exchange_addr = vm["exchange-addr"].as<std::string>();
        config.ticker = vm["ticker"].as<std::string>();
        config.side = (vm["side"].as<std::string>() == "buyer") ? Order::Side::BID : Order::Side::ASK;
        config.limit = vm["limit"].as<double>();
        config.delay = vm["delay"].as<unsigned int>();

        std::shared_ptr<TraderZIC> trader (new TraderZIC{&entity, &config});
        entity.setAgent(std::static_pointer_cast<Agent>(trader));
        entity.start();
    } 
    else if (agent_type == "shvr")
    {
        // Create configuration
        TraderConfig config;
        config.agent_id = agent_id;
        config.exchange_name = vm["exchange-name"].as<std::string>();
        config.exchange_addr = vm["exchange-addr"].as<std::string>();
        config.ticker = vm["ticker"].as<std::string>();
        config.side = (vm["side"].as<std::string>() == "buyer") ? Order::Side::BID : Order::Side::ASK;
        config.limit = vm["limit"].as<double>();
        config.delay = vm["delay"].as<unsigned int>();

        std::shared_ptr<TraderShaver> trader (new TraderShaver{&entity, &config});
        entity.setAgent(std::static_pointer_cast<Agent>(trader));
        entity.start();
    }
    else {
        std::cerr << "Invalid agent type: " << agent_type << "\n";
        std::cout << "\n" << showLocalUsage() << desc << std::endl;
        exit(1);
    } 
}

void cloud_runner(int argc, char** argv)
{
    po::options_description cloud_desc("Allowed options");
    cloud_desc.add_options()
        ("help", "show help message")
        ("port", po::value<unsigned short>()->default_value(8080), "set the port of the current agent")
    ;

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, cloud_desc), vm);
    po::notify(vm);

    if (vm.count("help"))
    {
        std::cout << "\n" << cloud_desc << std::endl;
        exit(1);
    }

    unsigned short port { vm["port"].as<unsigned short>() };

    asio::io_context io_context;
    NetworkEntity entity{io_context, port};
    entity.start();
}

void orchestrator(int argc, char** argv)
{
    // Create a local OrchestratorAgent

    asio::io_context io_context;
    NetworkEntity entity{io_context, 10001};

    AgentConfig orchestrator_config;
    orchestrator_config.agent_id = 999;

    std::shared_ptr<OrchestratorAgent> orchestrator (new OrchestratorAgent{&entity, &orchestrator_config});
    entity.setAgent(std::static_pointer_cast<Agent>(orchestrator));

    // === Configure the simulation ===

    std::vector<std::string> addresses {
        std::string{"18.171.57.85:8080"}, // Exchange
        std::string{"3.10.221.145:8080"},
        std::string{"35.178.174.204:8080"},
        std::string{"18.133.221.85:8080"},
        std::string{"3.9.115.201:8080"}
    };

    // 1. Create an Exchange

    /** TODO: Synchronisation: First create the exchange then continue with the rest. */

    ExchangeConfig exchange_config;
    exchange_config.agent_id = 99;
    exchange_config.addr = addresses.at(0);
    exchange_config.name = std::string{"NASDAQ"};
    exchange_config.tickers = std::vector{std::string{"AMZN"}};
    exchange_config.connect_time = 60;
    exchange_config.trading_time = 120;

    orchestrator->configureNode(exchange_config.addr, AgentType::STOCK_EXCHANGE, (AgentConfig*) &exchange_config);

    for (int i=0; i < 10000; i++)
    {
        std::cout << "Busy waiting for exchange.. i=" << i << std::endl;
        std::cout << "\033[2J\033[1;1H";
    }

    // 2. Create traders

    int sellers = 2;
    int buyers = 2;

    // Sellers ZIC
    for (int i=1; i < 1+sellers; i++)
    {
        TraderConfig* trader_config = new TraderConfig();
        trader_config->agent_id = i;
        trader_config->addr = addresses.at(i);
        trader_config->exchange_name = exchange_config.name;
        trader_config->exchange_addr = exchange_config.addr;
        trader_config->limit = 50;
        trader_config->delay = 0;
        trader_config->ticker = std::string{"AMZN"};
        trader_config->side = Order::Side::ASK;

        std::string trader_addr {addresses.at(i)};
        std::cout << "Configuring node with addr " << trader_addr << "\n"; 

        orchestrator->configureNode(trader_addr, AgentType::TRADER_ZIC, (AgentConfig*) trader_config);
    }

    // Buyers ZIC
    for (int i=1+sellers; i < 1+sellers+buyers; i++)
    {
        TraderConfig* trader_config = new TraderConfig();
        trader_config->agent_id = i;
        trader_config->addr = addresses.at(i);
        trader_config->exchange_name = exchange_config.name;
        trader_config->exchange_addr = exchange_config.addr;
        trader_config->limit = 150;
        trader_config->delay = 0;
        trader_config->ticker = std::string{"AMZN"};
        trader_config->side = Order::Side::BID;

        std::string trader_addr {addresses.at(i)};
        std::cout << "Configuring node with addr " << trader_addr << "\n";

        orchestrator->configureNode(trader_addr, AgentType::TRADER_ZIC, (AgentConfig*) trader_config);
    }

    // Sellers SHVR
    // for (int i=21; i < 25; i++)
    // {
    //     TraderConfig* trader_config = new TraderConfig();
    //     trader_config->agent_id = i;
    //     trader_config->addr = addresses.at(i);
    //     trader_config->exchange_name = exchange_config.name;
    //     trader_config->exchange_addr = exchange_config.addr;
    //     trader_config->limit = 50 + ((i-21)*20);
    //     trader_config->delay = 30;
    //     trader_config->ticker = std::string{"AMZN"};
    //     trader_config->side = Order::Side::ASK;

    //     std::string trader_addr {addresses.at(i)};
    //     std::cout << "Configuring node with addr " << trader_addr << "\n"; 

    //     orchestrator->configureNode(trader_addr, AgentType::TRADER_SHVR, (AgentConfig*) trader_config);
    // }

    // // Buyers SHVR
    // for (int i=25; i < 29; i++)
    // {
    //     TraderConfig* trader_config = new TraderConfig();
    //     trader_config->agent_id = i;
    //     trader_config->addr = addresses.at(i);
    //     trader_config->exchange_name = exchange_config.name;
    //     trader_config->exchange_addr = exchange_config.addr;
    //     trader_config->limit = 150 - ((i-25)*20);
    //     trader_config->delay = 30;
    //     trader_config->ticker = std::string{"AMZN"};
    //     trader_config->side = Order::Side::BID;

    //     std::string trader_addr {addresses.at(i)};
    //     std::cout << "Configuring node with addr " << trader_addr << "\n";

    //     orchestrator->configureNode(trader_addr, AgentType::TRADER_SHVR, (AgentConfig*) trader_config);
    // }

    entity.start();
}

int main(int argc, char** argv)
{

    if (argc < 2)
    {
        std::cout << showUsage() << std::endl;
        exit(1);
    }

    std::string mode { argv[1] };
    if (mode == "local")
    {
        local_runner(argc, argv);
    }
    else if (mode == "orchestrator")
    {
        orchestrator(argc, argv);
    }
    else
    {
        cloud_runner(argc, argv);
    }

    return 0;
}