#pragma once

#include "endpoint.h"
#include <fastrtps/rtps/attributes/WriterAttributes.h>
#include <fastrtps/qos/WriterQos.h>
#include <fastrtps/rtps/history/WriterHistory.h>
#include <fastrtps/rtps/writer/RTPSWriter.h>
#include <fastrtps/rtps/writer/WriterListener.h>

#include <memory>

namespace test
{

class Writer : public Endpoint
{
public:
    Writer();
    virtual ~Writer();
    void write(const uint8_t* pData, uint32_t dataLength);

private:
    eprosima::fastrtps::rtps::WriterAttributes writer_attr_;
    eprosima::fastrtps::WriterQos writer_qos_;
    std::unique_ptr<eprosima::fastrtps::rtps::WriterHistory> history_;
    eprosima::fastrtps::rtps::RTPSWriter* writer_;
};

}
