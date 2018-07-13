#ifndef MONITORMESSENGER_H
#define MONITORMESSENGER_H

#include "Messenger.h"
#include "LogMonitor.h"
#include "ControlMonitor.h"

#include <mutex>

///=====================================================================================================================
/// implements messenger for the remote monitoring system
///=====================================================================================================================
class MonitorMessenger : public Messenger
{
    friend class ControlInfoListener;

public:
    MonitorMessenger(int domainId);

    virtual ~MonitorMessenger();

    /// -------------- Control Parameters --------------------

    /// return topics available for remote control
    std::vector<std::string> controlVariableNames() const { return _controlTopics; }

    /// Attach a callback for acknowledgement of control updates.
    template<typename T>
    void monitorControlVariable(const std::string& name, const ControlMonitorCallback<T>& callback);

    /// Stop monitoring a control variable
    void unmonitorControlVariable(const std::string& name);

    /// Set a control variable that is being monitored
    template<typename T>
    void control(const std::string& name, const T& value);

    /// probe plant to send control variable info. The installed monitor callback will catch the reply
    void probeControlInfo(const std::string& topicName);

    /// probe plant to send all control variable info. Installed monitor callbacks will catch the replies.
    void probeAllControlInfo();

    ///-------------- Log Variables ---------------------------

    /// return log topics published by other participants within the network
    const std::vector<std::string> logVariableNames() const { return _logTopics; }

    /// monitor a topic. attach a message handler that gets called on every update on the topic
    template <typename T>
    void monitorLogVariable(const std::string& name, const LogMonitorCallback<T>& callback);

    /// stop monitoring a log variable
    void unmonitorLogVariable(const std::string& name);

private:
    dds::pub::Publisher     _publisher;
    dds::sub::Subscriber    _subscriber;

    dds::pub::DataWriter<dds::core::StringTopicType>        _controlProbeWriter; 
    dds::sub::DataReader<dds::core::KeyedBytesTopicType>    _controlInfoReader;

    std::vector<std::string> _logTopics;
    std::vector<std::string> _controlTopics;

    std::map<std::string, std::unique_ptr<AbstractLogMonitor> >     _logMonitors;
    std::map<std::string, std::unique_ptr<AbstractControlMonitor> > _controlMonitors;
    mutable std::mutex _controlMutex;

};

//---------------------------------------------------------------------------------------------------------------------
template<typename T>
void MonitorMessenger::monitorLogVariable(const std::string& topicName, const LogMonitorCallback<T>& fn)
//---------------------------------------------------------------------------------------------------------------------
{
    const auto& it = _logMonitors.find(topicName);
    if( it == _logMonitors.end() )
    {
        std::unique_ptr<AbstractLogMonitor> p(new LogMonitorT<T>(*_pParticipant, _subscriber, topicName, fn));
        _logMonitors.emplace(topicName, std::move(p));
    }
}

//---------------------------------------------------------------------------------------------------------------------
template<typename T>
void MonitorMessenger::monitorControlVariable(const std::string& name, const ControlMonitorCallback<T>& callback)
//---------------------------------------------------------------------------------------------------------------------
{
    std::unique_ptr<AbstractControlMonitor> p(new  ControlMonitorT<T>(*_pParticipant, _publisher, name, callback) );

    // setup callback
    _controlMonitors.emplace(name, std::move(p));

    // probe plant
    probeControlInfo(name);
}

//---------------------------------------------------------------------------------------------------------------------
template<typename T>
void MonitorMessenger::control(const std::string& name, const T& value)
//---------------------------------------------------------------------------------------------------------------------
{
    const auto& it = _controlMonitors.find(name);
    if(it != _controlMonitors.end())
    {
        ControlMonitorT<T>* p = dynamic_cast< ControlMonitorT<T>* >(_controlMonitors.at(name).get());
        p->control(value);
    }
}


#endif // MONITORMESSENGER_H
