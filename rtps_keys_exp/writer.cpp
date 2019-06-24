#include "writer.h"
#include "my_topic.h"

#include <fastrtps/qos/QosPolicies.h>
#include <fastrtps/rtps/RTPSDomain.h>

namespace test
{

//=====================================================================================================================
void Writer::Listener::onWriterMatched(eprosima::fastrtps::rtps::RTPSWriter* writer, eprosima::fastrtps::rtps::MatchingInfo& info)
//=====================================================================================================================
{
  (void)writer;

  // std::cout << "[WriterBase::Impl::Listener::onWriterMatched]:\n";
  switch (info.status)
  {
    case eprosima::fastrtps::rtps::MatchingStatus::MATCHED_MATCHING:  // new publisher/subscriber found
      // std::cout << "\tstatus: MATCHED_MATCHING\n";
      break;
    case eprosima::fastrtps::rtps::MatchingStatus::REMOVED_MATCHING:  // publisher/subscriber removed
      // std::cout << "\tstatus: REMOVED_MATCHING\n";
      break;
  };
}

//=====================================================================================================================
Writer::Writer() : Endpoint(0, "WRITER_NODE"), writer_(nullptr)
//=====================================================================================================================
{
  // set reliability qos
  switch (MyTopicAttributes::RELIABILITY_QOS)
  {
    case eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS:
      writer_attr_.endpoint.reliabilityKind = eprosima::fastrtps::rtps::ReliabilityKind_t::BEST_EFFORT;
      writer_qos_.m_reliability.kind = eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS;
      break;
    case eprosima::fastrtps::RELIABLE_RELIABILITY_QOS:
      writer_attr_.endpoint.reliabilityKind = eprosima::fastrtps::rtps::ReliabilityKind_t::RELIABLE;
      writer_qos_.m_reliability.kind = eprosima::fastrtps::RELIABLE_RELIABILITY_QOS;
      break;
  }

  // flow control
  writer_attr_.throughputController.bytesPerPeriod = MyTopicAttributes::DEFAULT_THROUGHPUT_BYTES_PER_PERIOD;
  writer_attr_.throughputController.periodMillisecs = MyTopicAttributes::DEFAULT_THROUGHPUT_PERIOD.count();

  // mode: asynchronous - FastRTPS insists on asynchronous mode if data flow rate is set
  writer_attr_.mode = eprosima::fastrtps::rtps::ASYNCHRONOUS_WRITER;
  writer_qos_.m_publishMode.kind = eprosima::fastrtps::ASYNCHRONOUS_PUBLISH_MODE;

  // heartbeat period of book-keeping messages (detects lost messages, etc).
  /// \note Smaller value increases network overhead, but provides faster response to data loss.
  constexpr auto SEC_TO_NANOSEC = 1000000000;
  constexpr uint32_t heartbeat_ms = 5000;
  const uint32_t heartbeat_sec = heartbeat_ms / 1000;
  writer_attr_.times.heartbeatPeriod.seconds = heartbeat_sec;
  const auto frac = heartbeat_sec - writer_attr_.times.heartbeatPeriod.seconds;
  writer_attr_.times.heartbeatPeriod.nanosec = static_cast<uint32_t>(frac * SEC_TO_NANOSEC);

  // durability
  switch (MyTopicAttributes::DURABILITY_QOS)
  {
    case eprosima::fastrtps::TRANSIENT_LOCAL_DURABILITY_QOS:
      writer_attr_.endpoint.durabilityKind = eprosima::fastrtps::rtps::TRANSIENT_LOCAL;
      writer_qos_.m_durability.kind = eprosima::fastrtps::TRANSIENT_LOCAL_DURABILITY_QOS;
      break;
    case eprosima::fastrtps::VOLATILE_DURABILITY_QOS:
      writer_attr_.endpoint.durabilityKind = eprosima::fastrtps::rtps::VOLATILE;
      writer_qos_.m_durability.kind = eprosima::fastrtps::VOLATILE_DURABILITY_QOS;
      break;
  }

    history_ = std::make_unique<eprosima::fastrtps::rtps::WriterHistory>(history_attr_);
    if (history_ == nullptr)
    {
      throw std::runtime_error("Creating RTPSWriterHistory failed");
    }

    writer_ = eprosima::fastrtps::rtps::RTPSDomain::createRTPSWriter(participant_, writer_attr_, history_.get(), &writer_listener_);
    if (writer_ == nullptr)
    {
      throw std::runtime_error("Creating createRTPSWriter failed");
    }

    if (!participant_->registerWriter(writer_, topic_attr_, writer_qos_))
    {
      eprosima::fastrtps::rtps::RTPSDomain::removeRTPSWriter(writer_);
      writer_ = nullptr;
      throw std::runtime_error("registerWriter failed");
    }

}

//=====================================================================================================================
Writer::~Writer()
//=====================================================================================================================
{
    if (writer_ != nullptr)
    {
      eprosima::fastrtps::rtps::RTPSDomain::removeRTPSWriter(writer_);
      writer_ = nullptr;
    }

}

//=====================================================================================================================
void Writer::write(const uint8_t* pData, uint32_t dataLength)
//=====================================================================================================================
{
  if (writer_ == nullptr)
  {
    throw std::runtime_error("Writer not initialised");
  }

  static constexpr uint32_t MAX_UNFRAGMENTED_PAYLOAD_SIZE = 64000;  /// Has to be less than UDP packet size

  const std::function<uint32_t()> dataSizeFunc = [dataLength]() -> uint32_t { return dataLength * sizeof(uint8_t); };

  eprosima::fastrtps::rtps::CacheChange_t* pchange = writer_->new_change(dataSizeFunc, eprosima::fastrtps::rtps::ChangeKind_t::ALIVE);

  // In the case history is full, remove some old changes
  /// \todo Review this. Implement a policy for what to do when history is full
  const uint32_t max_changes_to_remove = 1;
  uint32_t num_removed = 0;
  while (pchange == nullptr)
  {
    /// \note: I am assuming this removes the oldest changes, not the latest ones
    writer_->remove_older_changes(max_changes_to_remove);
    ++num_removed;
    pchange = writer_->new_change(dataSizeFunc, eprosima::fastrtps::rtps::ChangeKind_t::ALIVE);
  }

  // copy data into buffer and send
  pchange->serializedPayload.length = dataSizeFunc();
  if (pchange->serializedPayload.length > MAX_UNFRAGMENTED_PAYLOAD_SIZE)
  {
    pchange->setFragmentSize(MAX_UNFRAGMENTED_PAYLOAD_SIZE);
  }
  memcpy(pchange->serializedPayload.data, pData, dataLength);
  history_->add_change(pchange);
}

}
