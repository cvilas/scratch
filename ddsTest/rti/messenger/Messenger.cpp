#include "Messenger.h"
#include <dds/domain/qos/DomainParticipantFactoryQos.hpp>

#include <thread>

constexpr int64_t Messenger::BRINGUP_TIME_MS;
constexpr char Messenger::BUILTIN_TOPIC_CONTROL_INFO[];
constexpr char Messenger::BUILTIN_TOPIC_CONTROL_PROBE[];
constexpr char Messenger::BUILTIN_MESSAGE_CONTROL_PROBE_ALL[];

//=====================================================================================================================
Messenger::Messenger(int domainId)
//=====================================================================================================================
{   
    // start with participant disabled by default, so that we can install listeners first
    dds::domain::qos::DomainParticipantFactoryQos factoryQos = dds::domain::DomainParticipant::participant_factory_qos();
    factoryQos << dds::core::policy::EntityFactory::ManuallyEnable();
    dds::domain::DomainParticipant::participant_factory_qos(factoryQos);

    _pParticipant = std::make_shared<dds::domain::DomainParticipant>(domainId);
}

//---------------------------------------------------------------------------------------------------------------------
Messenger::~Messenger()
//---------------------------------------------------------------------------------------------------------------------
{
}

//---------------------------------------------------------------------------------------------------------------------
void Messenger::enable()
//---------------------------------------------------------------------------------------------------------------------
{
    _pParticipant->enable();

    // allow time for all elements to be up and running
    std::this_thread::sleep_for(std::chrono::milliseconds(BRINGUP_TIME_MS));
}


