#ifndef PLANTMESSENGER_H
#define PLANTMESSENGER_H

#include "Messenger.h"
#include "LogPublisher.h"
#include "ControlUpdater.h"

#include <map>
#include <memory> //unique_ptr
#include <mutex>

class AbstractLogPublisher;
class AbstractControlUpdater;

///=====================================================================================================================
/// implements messenger for the embedded system
///=====================================================================================================================
class PlantMessenger : public Messenger
{
    friend class ControlProbeListener;

public:
    PlantMessenger(int domainId);

    virtual ~PlantMessenger();

    /// -------------- Control Variables --------------------

    /// register a parameter that can be modified remotely
    template <typename T>
    void registerControlVariable(const std::string& name, T* pParamRef, T minVal, T maxVal);

    /// unregister a control variable
    void unregisterControlVariable(const std::string& name);

    /// Update control parameters with latest settings from the remote
    /// monitor and send acknowledgement to the remote monitor
    void updateAllControlVariables();

    /// Update specified control parameter and send acknowledgement to the remote monitor
    void updateControlVariable(const std::string& name);

    ///-------------- Log Variables ---------------------------

    /// register a signal that can be monitored remotely
    template <typename T>
    void registerLogVariable(const std::string& name, const T* pParamRef);

    /// unregister a log variable
    void unregisterLogVariable(const std::string& name);

    /// Send remote monitor the latest values for logged signals
    void transmitAllLogVariables();

    /// Send remote monitor the latest update for specified log signal
    void transmitLogVariable(const std::string& name);

private:
    void setupControlProbeListener();
    void transmitControlInfo(const AbstractControlUpdater& var);

private:
    mutable std::mutex                                              _controlVariablesMutex;
    std::map<std::string, std::unique_ptr<AbstractControlUpdater> > _controlVariables;
    std::map<std::string, std::unique_ptr<AbstractLogPublisher> >   _logVariables;

    dds::pub::Publisher     _publisher;
    dds::sub::Subscriber    _subscriber;

    dds::sub::DataReader<dds::core::StringTopicType>        _controlProbeReader;
    dds::pub::DataWriter<dds::core::KeyedBytesTopicType>    _controlInfoWriter;
};


//---------------------------------------------------------------------------------------------------------------------
template<typename T>
void PlantMessenger::registerControlVariable(const std::string& name, T* pParamRef, T minVal, T maxVal)
//---------------------------------------------------------------------------------------------------------------------
{
    auto p =
    std::unique_ptr<AbstractControlUpdater>(new ControlUpdaterT<T>(*_pParticipant, _subscriber,
                                                                   name, pParamRef, minVal, maxVal));
    std::lock_guard<std::mutex> guard(_controlVariablesMutex);
    _controlVariables.emplace(name, std::move(p));
    transmitControlInfo(*_controlVariables[name]);
}

//---------------------------------------------------------------------------------------------------------------------
template<typename T>
void PlantMessenger::registerLogVariable(const std::string& name, const T* pParamRef)
//---------------------------------------------------------------------------------------------------------------------
{
    auto p =  std::unique_ptr<AbstractLogPublisher>(new LogPublisherT<T>(*_pParticipant, _publisher, name, pParamRef));
    _logVariables.emplace(name, std::move(p));
}


#endif // PLANTMESSENGER_H
