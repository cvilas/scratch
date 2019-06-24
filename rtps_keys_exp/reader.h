#pragma once

#include "endpoint.h"
#include <fastrtps/rtps/attributes/ReaderAttributes.h>
#include <fastrtps/rtps/reader/ReaderListener.h>
#include <fastrtps/rtps/reader/RTPSReader.h>
#include <fastrtps/rtps/history/ReaderHistory.h>
#include <fastrtps/qos/ReaderQos.h>

namespace test
{

class Reader : public Endpoint
{
public:
    using ReaderCallback = std::function<void(uint8_t* const data, uint32_t dataLength)>;

    Reader(const ReaderCallback& fn);
    virtual ~Reader();

private:

    class Listener : public eprosima::fastrtps::rtps::ReaderListener
    {
    public:
        Listener(const ReaderCallback& fn);
        ~Listener() = default;
        void onReaderMatched(eprosima::fastrtps::rtps::RTPSReader* pReader, eprosima::fastrtps::rtps::MatchingInfo& info) final;
        void onNewCacheChangeAdded(eprosima::fastrtps::rtps::RTPSReader* pReader, const eprosima::fastrtps::rtps::CacheChange_t* const pChange) final;
    private:
        ReaderCallback callback_;
    };

private:
    eprosima::fastrtps::rtps::ReaderAttributes reader_attr_;
    eprosima::fastrtps::ReaderQos reader_qos_;
    Listener reader_listener_;
    std::unique_ptr<eprosima::fastrtps::rtps::ReaderHistory> history_;
    eprosima::fastrtps::rtps::RTPSReader* reader_;
};

} // test
