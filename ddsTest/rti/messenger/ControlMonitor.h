#ifndef CONTROLMONITOR_H
#define CONTROLMONITOR_H

#include "ControlVariable.h"
#include <dds/pub/DataWriter.hpp>


template<typename T>
using ControlMonitorCallback = std::function<void(const ControlVariableT<T>&)>;

//=====================================================================================================================
class AbstractControlMonitor
//=====================================================================================================================
{
public:
    virtual ~AbstractControlMonitor() = default;
    virtual void callback(const std::vector<uint8_t>& data) = 0;
};

//=====================================================================================================================
template<typename T>
class ControlMonitorT : public AbstractControlMonitor
//=====================================================================================================================
{
public:
    ControlMonitorT(dds::domain::DomainParticipant& participant, dds::pub::Publisher& publisher,
                    const std::string& name, const ControlMonitorCallback<T>& callback)
        : _topicName(name),
          _callback(callback),
          _writer(publisher, dds::topic::Topic<dds::core::KeyedBytesTopicType>(participant, name))
    {}

    ~ControlMonitorT()
    {
        std::cout << "[~ControlMonitorT (" << _topicName << ")]" << std::endl;
    }

    void callback(const std::vector<uint8_t>& data) final
    {
        _prototype.fromBytes(data);
        _callback(_prototype);
    }

    void control(const T& value)
    {
        size_t len = sizeof(T);
        std::vector<uint8_t> bytes(len);

        const uint8_t* v = reinterpret_cast<const uint8_t*>(&value);
        memcpy(&bytes[0], v, len);

        dds::core::KeyedBytesTopicType data(_topicName, bytes);
        _writer.write(data);
    }

private:
    const std::string _topicName;
    ControlVariableT<T> _prototype;
    ControlMonitorCallback<T> _callback;
    dds::pub::DataWriter<dds::core::KeyedBytesTopicType> _writer;

};

#endif // CONTROLMONITOR_H
