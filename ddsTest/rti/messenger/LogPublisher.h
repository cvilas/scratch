#ifndef LOGPUBLISHER_H
#define LOGPUBLISHER_H

#include "LogVariable.h"
#include <dds/pub/Publisher.hpp>
#include <dds/pub/DataWriter.hpp>

//=====================================================================================================================
class AbstractLogPublisher
//=====================================================================================================================
{
public:
    virtual void publish() = 0;
    virtual ~AbstractLogPublisher() = default;
};

//=====================================================================================================================
template <typename T>
class LogPublisherT : public AbstractLogPublisher
//=====================================================================================================================
{
public:
    LogPublisherT(dds::domain::DomainParticipant& participant, dds::pub::Publisher& publisher,
                    const std::string& topicName, const T* pVar)
        : _topicName(topicName),
          _writer(publisher, dds::topic::Topic<dds::core::KeyedBytesTopicType>(participant, topicName)),
          _pNakedPointer(pVar)
    {
    }

    ~LogPublisherT()
    {
        std::cout << "[~LogPublisherT (" << _topicName << ")]" << std::endl;
    }

    void publish() final
    {
        LogVariableT<T> variable(*_pNakedPointer);
        dds::core::KeyedBytesTopicType data(_topicName, variable.toBytes());
        _writer.write(data);
    }

private:
    const std::string _topicName;
    dds::pub::DataWriter<dds::core::KeyedBytesTopicType> _writer;
    const T* _pNakedPointer;
};

#endif // LOGPUBLISHER_H
