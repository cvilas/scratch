#include "reader.h"
#include "my_topic.h"
#include "my_type.h"

#include <fastrtps/rtps/RTPSDomain.h>
#include <memory>

namespace test
{

//=====================================================================================================================
Reader::Listener::Listener(const ReaderCallback& fn) : callback_(fn)
//=====================================================================================================================
{}

//=====================================================================================================================
void Reader::Listener::onReaderMatched(eprosima::fastrtps::rtps::RTPSReader* pReader, eprosima::fastrtps::rtps::MatchingInfo& info)
//=====================================================================================================================
{
  (void)pReader;
  (void)info;
}

//---------------------------------------------------------------------------------------------------------------------
void Reader::Listener::onNewCacheChangeAdded(eprosima::fastrtps::rtps::RTPSReader* pReader, const eprosima::fastrtps::rtps::CacheChange_t* const pChange)
//---------------------------------------------------------------------------------------------------------------------
{
  std::cout << __FUNCTION__ << " ih: " << (int)(pChange->instanceHandle.value[0]) << "\n";
  if (callback_)
  {
    callback_(pChange->serializedPayload.data, pChange->serializedPayload.length);
  }
  pReader->getHistory()->remove_change(const_cast<eprosima::fastrtps::rtps::CacheChange_t*>(pChange));
}

//=====================================================================================================================
Reader::Reader(const ReaderCallback& fn) : Endpoint(0, "READER_NODE"), reader_listener_(fn), reader_(nullptr)
//=====================================================================================================================
{
  /// \todo
  /// - Set all fields in reader_attr_, reader_qos_, history_, topic_attr_
  /// - Set all fields in writer_attr_, writer_qos_, history_

  /// Change 1: Set endpoint topic kind
  test::MyTopicAttributes::HAS_KEY
      ? reader_attr_.endpoint.topicKind = eprosima::fastrtps::rtps::TopicKind_t::WITH_KEY
      : reader_attr_.endpoint.topicKind = eprosima::fastrtps::rtps::TopicKind_t::NO_KEY;

  // set reliability qos
  switch (MyTopicAttributes::RELIABILITY_QOS)
  {
    case eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS:
      reader_attr_.endpoint.reliabilityKind = eprosima::fastrtps::rtps::ReliabilityKind_t::BEST_EFFORT;
      reader_qos_.m_reliability.kind = eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS;
      break;
    case eprosima::fastrtps::RELIABLE_RELIABILITY_QOS:
      reader_attr_.endpoint.reliabilityKind = eprosima::fastrtps::rtps::ReliabilityKind_t::RELIABLE;
      reader_qos_.m_reliability.kind = eprosima::fastrtps::RELIABLE_RELIABILITY_QOS;
      break;
  }

  // Settings to acknowledge writer heartbeats
  /// \note Smaller delay increases network overhead, but provides faster response to data loss.
  constexpr auto SEC_TO_NANOSEC = 1000000000;
  constexpr uint32_t heartbeat_ms = 5000;
  const uint32_t heartbeat_sec = heartbeat_ms / 1000;
  reader_attr_.times.heartbeatResponseDelay.seconds = heartbeat_sec;
  const auto frac = heartbeat_sec - reader_attr_.times.heartbeatResponseDelay.seconds;
  reader_attr_.times.heartbeatResponseDelay.nanosec = static_cast<uint32_t>(frac * SEC_TO_NANOSEC);

  // durability
  switch (MyTopicAttributes::DURABILITY_QOS)
  {
    case eprosima::fastrtps::TRANSIENT_LOCAL_DURABILITY_QOS:
      reader_attr_.endpoint.durabilityKind = eprosima::fastrtps::rtps::TRANSIENT_LOCAL;
      reader_qos_.m_durability.kind = eprosima::fastrtps::TRANSIENT_LOCAL_DURABILITY_QOS;
      break;
    case eprosima::fastrtps::VOLATILE_DURABILITY_QOS:
      reader_attr_.endpoint.durabilityKind = eprosima::fastrtps::rtps::VOLATILE;
      reader_qos_.m_durability.kind = eprosima::fastrtps::VOLATILE_DURABILITY_QOS;
      break;
  }

    reader_history_ = std::make_unique<eprosima::fastrtps::rtps::ReaderHistory>(history_attr_);
    if (reader_history_ == nullptr)
    {
      throw std::runtime_error("Creating ReaderHistory failed");
    }

    reader_ = eprosima::fastrtps::rtps::RTPSDomain::createRTPSReader(participant_, reader_attr_, reader_history_.get(), &reader_listener_);
    if (reader_ == nullptr)
    {
      throw std::runtime_error("Creating createRTPSReader failed");
    }

    if (!participant_->registerReader(reader_, topic_attr_, reader_qos_))
    {
      eprosima::fastrtps::rtps::RTPSDomain::removeRTPSReader(reader_);
      reader_ = nullptr;
      throw std::runtime_error("registerReader failed");
    }

}

//=====================================================================================================================
Reader::~Reader()
//=====================================================================================================================
{
    if (reader_ != nullptr)
    {
      eprosima::fastrtps::rtps::RTPSDomain::removeRTPSReader(reader_);
    }
}

} // test
