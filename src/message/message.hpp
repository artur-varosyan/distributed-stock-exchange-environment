#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include <chrono>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/unordered_map.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/export.hpp>

#include "messagetype.hpp"

/** A message that can be sent between network entities. */
class Message : std::enable_shared_from_this<Message>
{
public:

    Message() = default;

    Message(MessageType type) 
    : type{type}
    {
        std::chrono::system_clock::duration now = std::chrono::system_clock::now().time_since_epoch();
        timestamp = std::chrono::duration_cast<std::chrono::nanoseconds>(now).count();
    };

    virtual ~Message() = default;

    /** Returns the size of the serialized messaged in bytes. */
    size_t getSerializedSize()
    {
        std::ostringstream oss;
        boost::archive::text_oarchive oa{oss};
        oa << *this;
        return oss.str().size();
    }

    int sender_id;
    // std::vector<int> receiver_ids;
    MessageType type;

    /** TODO: Split timestamp into sent and received to measure latency */
    unsigned long long timestamp;

private:

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {

        ar & type;
        ar & sender_id;
        // ar & receiver_ids;

        ar & timestamp;
    }

};

#endif