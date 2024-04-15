#ifndef MARKET_WATCHER_CONFIG_HPP
#define MARKET_WATCHER_CONFIG_HPP

#include "agentconfig.hpp"
#include "../order/order.hpp"

class MarketWatcherConfig : public AgentConfig
{
public:

    MarketWatcherConfig() = default;

    std::string exchange_name;
    std::string exchange_addr;
    std::string ticker;

private:
    
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & boost::serialization::base_object<AgentConfig>(*this);
        ar & exchange_name;
        ar & exchange_addr;
        ar & ticker;
    }

};

typedef std::shared_ptr<MarketWatcherConfig> MarketWatcherConfigPtr;

#endif