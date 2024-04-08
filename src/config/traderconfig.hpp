#ifndef TRADER_CONFIG_HPP
#define TRADER_CONFIG_HPP

#include "agentconfig.hpp"
#include "../order/order.hpp"

class TraderConfig : public AgentConfig
{
public:

    TraderConfig() = default;

    std::string exchange_name;
    std::string exchange_addr;
    std::string ticker;
    Order::Side side;
    double limit;
    unsigned int delay;

private:
    
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & boost::serialization::base_object<AgentConfig>(*this);
        ar & exchange_name;
        ar & exchange_addr;
        ar & ticker;
        ar & side;
        ar & limit;
        ar & delay;
    }

};

typedef std::shared_ptr<TraderConfig> TraderConfigPtr;

#endif