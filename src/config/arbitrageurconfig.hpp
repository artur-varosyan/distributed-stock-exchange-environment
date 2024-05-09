#ifndef ARBITRAGEUR_CONFIG_HPP
#define ARBITRAGEUR_CONFIG_HPP

#include "agentconfig.hpp"
#include "../order/order.hpp"

class ArbitrageurConfig : public AgentConfig
{
public:

    ArbitrageurConfig() = default;

    std::string exchange0_name;
    std::string exchange0_addr;
    std::string exchange1_name;
    std::string exchange1_addr;
    std::string ticker;
    unsigned int delay;
    double alpha;
    unsigned int trade_interval;
    bool cancelling;

private:
    
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & boost::serialization::base_object<AgentConfig>(*this);
        ar & exchange0_name;
        ar & exchange0_addr;
        ar & exchange1_name;
        ar & exchange1_addr;
        ar & ticker;
        ar & delay;
        ar & alpha;
        ar & trade_interval;
        ar & cancelling;
    }

};

typedef std::shared_ptr<ArbitrageurConfig> ArbitrageurConfigPtr;

#endif