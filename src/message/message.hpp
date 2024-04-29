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
#include "../utilities/csvprintable.hpp"

/** A message that can be sent between network entities. */
class Message : std::enable_shared_from_this<Message>, public CSVPrintable 
{
public:

    Message() = default;

    Message(MessageType type) 
    : type{type} {};

    virtual ~Message() = default;

    /** Returns the size of the serialized messaged in bytes. */
    size_t getSerializedSize()
    {
        std::ostringstream oss;
        boost::archive::text_oarchive oa{oss};
        oa << *this;
        return oss.str().size();
    }

    /** Marks the given message as sent and adds timestamp. */
    void markSent(int sender_id)
    {
        this->sender_id = sender_id;
        std::chrono::system_clock::duration now = std::chrono::system_clock::now().time_since_epoch();
        timestamp_sent = std::chrono::duration_cast<std::chrono::nanoseconds>(now).count();
    }

    /** Marks the given message as received and adds timestamp. */
    void markReceived()
    {
        std::chrono::system_clock::duration now = std::chrono::system_clock::now().time_since_epoch();
        timestamp_received = std::chrono::duration_cast<std::chrono::nanoseconds>(now).count();
    }

    /** Marks the given message as processed and adds timestamp. */
    void markProcessed()
    {
        std::chrono::system_clock::duration now = std::chrono::system_clock::now().time_since_epoch();
        timestamp_processed = std::chrono::duration_cast<std::chrono::nanoseconds>(now).count();
    }

    std::string describeCSVHeaders() const override
    {
        return "sender_id,timestamp_sent,timestamp_received,timestamp_processed";
    }

    std::string toCSV() const override
    {
        return std::to_string(sender_id) + ","  + std::to_string(timestamp_sent) + "," + std::to_string(timestamp_received) + "," + std::to_string(timestamp_processed);
    }

    MessageType type;
    int sender_id;
    unsigned long long timestamp_sent;
    unsigned long long timestamp_received;
    unsigned long long timestamp_processed;

private:

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & boost::serialization::base_object<CSVPrintable>(*this);
        ar & type;
        ar & sender_id;
        ar & timestamp_sent;
        ar & timestamp_received;
        ar & timestamp_processed;
    }

};

typedef std::shared_ptr<Message> MessagePtr;

#endif