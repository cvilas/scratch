#include "endpoint.h"

#include <fastrtps/transport/UDPv4TransportDescriptor.h>
#include <fastrtps/rtps/RTPSDomain.h>

#include <memory>

namespace test
{

//=====================================================================================================================
Endpoint::Endpoint(int domain, const std::string& name) : participant_(nullptr), listener_(*this)
//=====================================================================================================================
{
    auto transport = std::make_shared<eprosima::fastrtps::rtps::UDPv4TransportDescriptor>();
    transport->TTL = 1;  // by default span the local network

    // set participant attributes we care about
    eprosima::fastrtps::rtps::RTPSParticipantAttributes participant_params_;
    participant_params_.setName(name.c_str());
    participant_params_.builtin.domainId = domain;
    participant_params_.userTransports.push_back(transport);
    participant_params_.sendSocketBufferSize = 0;
    participant_params_.listenSocketBufferSize = 0;
    participant_params_.use_IP4_to_send = true;
    participant_params_.use_IP6_to_send = false;
    participant_params_.useBuiltinTransports = false;
    participant_params_.builtin.leaseDuration_announcementperiod.seconds = DISCOVERY_ANNOUNCE_PERIOD.count();
    participant_params_.builtin.leaseDuration.seconds = 2 * participant_params_.builtin.leaseDuration_announcementperiod.seconds;

    participant_ = eprosima::fastrtps::rtps::RTPSDomain::createParticipant(participant_params_, &listener_);
    if (participant_ == nullptr)
    {
      throw std::runtime_error("RTPSDomain::createParticipant failed");
    }

    history_attr_.memoryPolicy = eprosima::fastrtps::rtps::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;
    history_attr_.payloadMaxSize = 384;
    history_attr_.initialReservedCaches = 500;
    history_attr_.maximumReservedCaches = 0;

    topic_attr_.topicKind = eprosima::fastrtps::rtps::TopicKind_t::NO_KEY;
    topic_attr_.topicDataType = "TEST_TOPIC_TYPE";
    topic_attr_.topicName = "TEST_TOPIC";

    topic_attr_.historyQos.depth = 1;
    topic_attr_.historyQos.kind = eprosima::fastrtps::KEEP_LAST_HISTORY_QOS;

    topic_attr_.resourceLimitsQos.max_samples = 0;
    topic_attr_.resourceLimitsQos.max_instances = 500;
    topic_attr_.resourceLimitsQos.max_samples_per_instance = 0;
    topic_attr_.resourceLimitsQos.allocated_samples = 0;
}

//=====================================================================================================================
Endpoint::~Endpoint()
//=====================================================================================================================
{
    eprosima::fastrtps::rtps::RTPSDomain::removeRTPSParticipant(participant_);
}

//=====================================================================================================================
Endpoint::ParticipantListener::ParticipantListener(Endpoint& n) : endpoint_(n)
//=====================================================================================================================
{
}

//=====================================================================================================================
void Endpoint::ParticipantListener::onRTPSParticipantDiscovery(eprosima::fastrtps::rtps::RTPSParticipant* pPart, eprosima::fastrtps::rtps::RTPSParticipantDiscoveryInfo info)
//=====================================================================================================================
{
  // std::cout << "[NodeImpl::Listener::onRTPSParticipantDiscovery]:\n";
  // std::cout << "\tMy Name: " << pPart->getRTPSParticipantAttributes().getName() << "\n";

  (void)pPart;

  switch (info.m_status)
  {
    case eprosima::fastrtps::rtps::DISCOVERED_RTPSPARTICIPANT:
    {
      std::cout << "\tstatus: DISCOVERED_RTPSPARTICIPANT: \"" << info.m_RTPSParticipantName << "\"\n";
      break;
    }
    case eprosima::fastrtps::rtps::CHANGED_QOS_RTPSPARTICIPANT:
    {
      std::cout << "\tstatus: CHANGED_QOS_RTPSPARTICIPANT: \"" << info.m_RTPSParticipantName << "\"\n";
      break;
    }
    case eprosima::fastrtps::rtps::REMOVED_RTPSPARTICIPANT:
    {
      std::cout << "\tstatus: REMOVED_RTPSPARTICIPANT: \"" << info.m_RTPSParticipantName << "\"\n";
      break;
    }
    case eprosima::fastrtps::rtps::DROPPED_RTPSPARTICIPANT:
    {
      std::cout << "\tstatus: DROPPED_RTPSPARTICIPANT: \"" << info.m_RTPSParticipantName << "\"\n";
      break;
    }
    default:
      // std::cout << "\tstatus: UNKNOWN (ERROR)\n";
      break;
  }
}

constexpr std::chrono::seconds Endpoint::DISCOVERY_ANNOUNCE_PERIOD;

} // test
