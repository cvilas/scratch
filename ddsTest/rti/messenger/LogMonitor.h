#ifndef LOGMONITOR_H
#define LOGMONITOR_H

#include "LogVariable.h"
#include <dds/sub/Subscriber.hpp>
#include <dds/sub/DataReader.hpp>
#include <dds/topic/BuiltinTopic.hpp>
#include <dds/sub/DataReaderListener.hpp>

template<typename T>
using LogMonitorCallback = std::function<void(const LogVariableT<T>&)>;

//=====================================================================================================================
/// Listen for a log message
template<typename T>
class LogListenerT : public dds::sub::NoOpDataReaderListener<dds::core::KeyedBytesTopicType>
//=====================================================================================================================
{
public:
    LogListenerT(const LogMonitorCallback<T>& callback) : _callback(callback) {}
    ~LogListenerT() {}
    void on_data_available(dds::sub::DataReader<dds::core::KeyedBytesTopicType>& reader) final;
private:
    LogMonitorCallback<T> _callback;
};


//=====================================================================================================================
class AbstractLogMonitor
//=====================================================================================================================
{
public:
    virtual ~AbstractLogMonitor() = default;
};


//=====================================================================================================================
template<typename T>
class LogMonitorT :  public AbstractLogMonitor
//=====================================================================================================================
{
public:
    LogMonitorT(dds::domain::DomainParticipant& participant, dds::sub::Subscriber& subscriber,
                const std::string & topicName, const LogMonitorCallback<T>& callback)
        : _topicName(topicName),
          _reader(subscriber, dds::topic::Topic<dds::core::KeyedBytesTopicType>(participant, topicName)),
          _listener(callback)
    {
        _reader.listener(&_listener, dds::core::status::StatusMask::data_available());
    }

    ~LogMonitorT()
    {
        _reader.close();
        std::cout << "[~LogMonitorT (" << _topicName << ")]" << std::endl;
    }


private:
    const std::string _topicName;
    dds::sub::DataReader<dds::core::KeyedBytesTopicType> _reader;
    LogListenerT<T> _listener;
};

//---------------------------------------------------------------------------------------------------------------------
template<typename T>
void LogListenerT<T>::on_data_available(dds::sub::DataReader<dds::core::KeyedBytesTopicType>& reader)
//---------------------------------------------------------------------------------------------------------------------
{
    dds::sub::LoanedSamples<dds::core::KeyedBytesTopicType> samples = reader.take();

    for( const auto& sample : samples )
    {
        if (sample.info().valid())
        {
            const dds::core::KeyedBytesTopicType& data = static_cast<const dds::core::KeyedBytesTopicType&>(sample.data());
            const std::string key = data.key().to_std_string();
            //std::cout << "[LogListenerT (" << key << ")]: received bytes" << std::endl;

            LogVariableT<T> logSample;
            logSample.fromBytes(data.value());
            _callback(logSample);
        }
    }
}

#endif // LOGMONITOR_H
