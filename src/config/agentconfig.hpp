#ifndef AGENT_CONFIG_HPP
#define AGENT_CONFIG_HPP

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/unordered_map.hpp>
#include <boost/serialization/vector.hpp>

#include "../agent/agenttype.hpp"

/** Used to configure an instance of an agent in the simulation. */
class AgentConfig : std::enable_shared_from_this<AgentConfig>
{
public:

    AgentConfig() = default;
    virtual ~AgentConfig() = default;

    int agent_id;
    std::string addr;
    AgentType type;

private:

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & agent_id;
        ar & addr;
        ar & type;
    }
};

typedef std::shared_ptr<AgentConfig> AgentConfigPtr;

#endif