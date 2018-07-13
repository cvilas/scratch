#include "PlantMessenger.h"
#include "ControlVariable.h"
#include "LogVariable.h"

#include <rti/core/ListenerBinder.hpp>
#include <dds/topic/Topic.hpp>
#include <dds/topic/BuiltinTopic.hpp>

//=====================================================================================================================
/// Listen for probe signal for specific control variable from the monitor, reply with control variable settings
class ControlProbeListener :  public dds::sub::NoOpDataReaderListener<dds::core::StringTopicType>
//=====================================================================================================================
{
public:
    ControlProbeListener(PlantMessenger& messenger)
        : _messenger(messenger) {}

    ~ControlProbeListener()
    {
        std::cout << "[~ControlProbeListener]" << std::endl;
    }

    void on_data_available(dds::sub::DataReader<dds::core::StringTopicType>& reader) final
    {
        dds::sub::LoanedSamples<dds::core::StringTopicType> samples = reader.take();

        for (const auto& sample : samples)
        {
            if (sample.info().valid())
            {
                const dds::core::StringTopicType& data = static_cast<const dds::core::StringTopicType&>(sample.data());
                std::string probedVariableName = data.data().to_std_string();
                //std::cout << "[ControlProbeListener] received probe for: " << probedVariableName << std::endl;

                std::lock_guard<std::mutex> guard(_messenger._controlVariablesMutex);
                if(probedVariableName == Messenger::BUILTIN_MESSAGE_CONTROL_PROBE_ALL)
                {
                    for(const auto& variable : _messenger._controlVariables)
                    {
                        _messenger.transmitControlInfo(*variable.second);
                    }
                }
                else
                {
                    const auto& it = _messenger._controlVariables.find(probedVariableName);
                    if( it != _messenger._controlVariables.end() )
                    {
                        _messenger.transmitControlInfo(*(it->second));
                    } // found variable
                }
            } // valid sample
        } // for each sample
    }
private:
    PlantMessenger& _messenger;
};


//---------------------------------------------------------------------------------------------------------------------
void PlantMessenger::unregisterControlVariable(const std::string& name)
//---------------------------------------------------------------------------------------------------------------------
{
    std::lock_guard<std::mutex> guard(_controlVariablesMutex);
    _controlVariables.erase(name);
}

//---------------------------------------------------------------------------------------------------------------------
void PlantMessenger::updateAllControlVariables()
//---------------------------------------------------------------------------------------------------------------------
{
    std::lock_guard<std::mutex> guard(_controlVariablesMutex);
    for(const auto& nameValPair : _controlVariables)
    {
        // update the variable in the plant
        if( nameValPair.second->updatePlant() )
        {
            // update status at the monitor
            transmitControlInfo(*nameValPair.second);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void PlantMessenger::updateControlVariable(const std::string& name)
//---------------------------------------------------------------------------------------------------------------------
{
    std::lock_guard<std::mutex> guard(_controlVariablesMutex);

    const auto& it = _controlVariables.find(name);
    if( it != _controlVariables.end() )
    {
        // update the variable in the plant
        AbstractControlUpdater& var = *(it->second);
        if( var.updatePlant() )
        {
            // update status at the monitor
            transmitControlInfo(var);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void PlantMessenger::setupControlProbeListener()
//---------------------------------------------------------------------------------------------------------------------
{
    // Making the following static ensures that the listener and the reader lives as long as the
    // messenger lives. When the messenger dies, the listener binder ensures that the listener is reset on the
    // reader, the listener is deleted and the reader is deleted

    static rti::core::ListenerBinder<dds::sub::DataReader<dds::core::StringTopicType> > controlProbeListener =
            rti::core::bind_and_manage_listener(_controlProbeReader,
                                                new ControlProbeListener(*this),
                                                dds::core::status::StatusMask::data_available());
}

//---------------------------------------------------------------------------------------------------------------------
void PlantMessenger::transmitControlInfo(const AbstractControlUpdater& variable)
//---------------------------------------------------------------------------------------------------------------------
{
    dds::core::KeyedBytesTopicType reply;
    reply.key(variable.name());
    reply.value(variable.toBytes());
    _controlInfoWriter.write(reply);
}

//=====================================================================================================================
PlantMessenger::PlantMessenger(int domainId)
//=====================================================================================================================
    : Messenger(domainId),
      _publisher(*_pParticipant),
      _subscriber(*_pParticipant),
      _controlProbeReader(_subscriber, dds::topic::Topic<dds::core::StringTopicType>(*_pParticipant, Messenger::BUILTIN_TOPIC_CONTROL_PROBE)),
      _controlInfoWriter(_publisher, dds::topic::Topic<dds::core::KeyedBytesTopicType>(*_pParticipant, Messenger::BUILTIN_TOPIC_CONTROL_INFO))

{
    setupControlProbeListener();
    enable();
}


//---------------------------------------------------------------------------------------------------------------------
PlantMessenger::~PlantMessenger()
//---------------------------------------------------------------------------------------------------------------------
{}


//---------------------------------------------------------------------------------------------------------------------
void PlantMessenger::unregisterLogVariable(const std::string& name)
//---------------------------------------------------------------------------------------------------------------------
{
    _logVariables.erase(name);
}

//---------------------------------------------------------------------------------------------------------------------
void PlantMessenger::transmitAllLogVariables()
//---------------------------------------------------------------------------------------------------------------------
{
    for(const auto& nameValPair : _logVariables)
    {
        nameValPair.second->publish();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void PlantMessenger::transmitLogVariable(const std::string& name)
//---------------------------------------------------------------------------------------------------------------------
{
    _logVariables[name]->publish();
}
