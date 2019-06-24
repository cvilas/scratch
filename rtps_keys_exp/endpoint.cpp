#include "endpoint.h"
#include "my_topic.h"
#include "my_type.h"

#include <fastrtps/transport/UDPv4TransportDescriptor.h>
#include <fastrtps/rtps/RTPSDomain.h>

#include <memory>

namespace test
{

//=====================================================================================================================
Endpoint::Endpoint(int domain, const std::string& name) : participant_(nullptr), participant_listener_(*this)
//=====================================================================================================================
{
    // set participant attributes we care about
    eprosima::fastrtps::rtps::RTPSParticipantAttributes part_attr_;
    part_attr_.setName(name.c_str());
    part_attr_.builtin.domainId = domain;

    part_attr_.builtin.use_SIMPLE_RTPSParticipantDiscoveryProtocol = true;
    part_attr_.builtin.use_SIMPLE_EndpointDiscoveryProtocol = true;
    part_attr_.builtin.m_simpleEDP.use_PublicationReaderANDSubscriptionWriter = true;
    part_attr_.builtin.m_simpleEDP.use_PublicationWriterANDSubscriptionReader = true;

    part_attr_.builtin.leaseDuration_announcementperiod.seconds = DISCOVERY_ANNOUNCE_PERIOD.count();
    part_attr_.builtin.leaseDuration.seconds = 2 * part_attr_.builtin.leaseDuration_announcementperiod.seconds;

    participant_ = eprosima::fastrtps::rtps::RTPSDomain::createParticipant(part_attr_, &participant_listener_);
    if (participant_ == nullptr)
    {
      throw std::runtime_error("RTPSDomain::createParticipant failed");
    }
    part_attr_ = participant_->getRTPSParticipantAttributes();  // updated attributes

    // set topic and history attributes
    const auto& num_instances = MyTopicAttributes::DEFAULT_NUM_INSTANCES;
    const auto& num_samples_per_instance = MyTopicAttributes::DEFAULT_NUM_SAMPLES_PER_INSTANCE;
    const auto num_samples = num_instances * num_samples_per_instance;

    topic_attr_.topicKind = eprosima::fastrtps::rtps::TopicKind_t::NO_KEY;
    topic_attr_.topicDataType = test::MyTopicAttributes::DATA_TYPE;
    topic_attr_.topicName = test::MyTopicAttributes::TOPIC;

    /// \todo Requires review
    topic_attr_.historyQos.depth = 1;  /// replace with a more recent CacheChange if available. See rtps v2.2 doc.
    topic_attr_.historyQos.kind = eprosima::fastrtps::KEEP_LAST_HISTORY_QOS;

    topic_attr_.resourceLimitsQos.max_samples = num_samples;  /// \note 0=>infinite. Allocate until something breaks
    topic_attr_.resourceLimitsQos.max_instances = num_instances;
    topic_attr_.resourceLimitsQos.max_samples_per_instance = num_samples_per_instance;
    topic_attr_.resourceLimitsQos.allocated_samples = num_samples;

    // set history attributes
    history_attr_.memoryPolicy = eprosima::fastrtps::rtps::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;
    history_attr_.payloadMaxSize = sizeof(MyType);
    history_attr_.initialReservedCaches = num_samples;
    history_attr_.maximumReservedCaches = num_samples;
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
void Endpoint::ParticipantListener::onParticipantDiscovery(eprosima::fastrtps::rtps::RTPSParticipant* pPart, eprosima::fastrtps::rtps::ParticipantDiscoveryInfo&& info)
//=====================================================================================================================
{
  // std::cout << "[NodeImpl::Listener::onRTPSParticipantDiscovery]:\n";
  // std::cout << "\tMy Name: " << pPart->getRTPSParticipantAttributes().getName() << "\n";

  (void)pPart;

  switch (info.status)
  {
    case eprosima::fastrtps::rtps::ParticipantDiscoveryInfo::DISCOVERED_PARTICIPANT:
    {
      std::cout << "\tstatus: DISCOVERED_RTPSPARTICIPANT: \"" << info.info.m_participantName << "\"\n";
      break;
    }
    case eprosima::fastrtps::rtps::ParticipantDiscoveryInfo::CHANGED_QOS_PARTICIPANT:
    {
      std::cout << "\tstatus: CHANGED_QOS_RTPSPARTICIPANT: \"" << info.info.m_participantName << "\"\n";
      break;
    }
    case eprosima::fastrtps::rtps::ParticipantDiscoveryInfo::REMOVED_PARTICIPANT:
    {
      std::cout << "\tstatus: REMOVED_RTPSPARTICIPANT: \"" << info.info.m_participantName << "\"\n";
      break;
    }
    case eprosima::fastrtps::rtps::ParticipantDiscoveryInfo::DROPPED_PARTICIPANT:
    {
      std::cout << "\tstatus: DROPPED_RTPSPARTICIPANT: \"" << info.info.m_participantName << "\"\n";
      break;
    }
  }
}

constexpr std::chrono::seconds Endpoint::DISCOVERY_ANNOUNCE_PERIOD;

} // test
