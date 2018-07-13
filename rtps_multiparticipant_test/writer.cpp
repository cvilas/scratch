#include "writer.h"

#include <fastrtps/qos/QosPolicies.h>
#include <fastrtps/rtps/RTPSDomain.h>

namespace test
{

//=====================================================================================================================
Writer::Writer() : Endpoint(0, "WRITER_NODE"), writer_(nullptr)
//=====================================================================================================================
{
    writer_attr_.mode = eprosima::fastrtps::rtps::SYNCHRONOUS_WRITER;
    writer_attr_.endpoint.reliabilityKind = eprosima::fastrtps::rtps::ReliabilityKind_t::RELIABLE;
    writer_attr_.endpoint.durabilityKind = eprosima::fastrtps::rtps::VOLATILE;

    constexpr uint32_t heartbeat_ms = 5000;
    const uint32_t heartbeat_sec = heartbeat_ms / 1000;
    writer_attr_.times.heartbeatPeriod.seconds = heartbeat_sec;
    writer_attr_.times.heartbeatPeriod.fraction = 4294967 * (heartbeat_ms - (heartbeat_sec * 1000));

    writer_qos_.m_publishMode.kind = eprosima::fastrtps::SYNCHRONOUS_PUBLISH_MODE;
    writer_qos_.m_reliability.kind = eprosima::fastrtps::RELIABLE_RELIABILITY_QOS;
    writer_qos_.m_durability.kind = eprosima::fastrtps::VOLATILE_DURABILITY_QOS;
    writer_qos_.m_liveliness.kind = eprosima::fastrtps::AUTOMATIC_LIVELINESS_QOS;
    writer_qos_.m_liveliness.announcement_period.seconds = Endpoint::DISCOVERY_ANNOUNCE_PERIOD.count();
    writer_qos_.m_liveliness.lease_duration.seconds = 10 * writer_qos_.m_liveliness.announcement_period.seconds;

    history_ = std::make_unique<eprosima::fastrtps::rtps::WriterHistory>(history_attr_);
    if (history_ == nullptr)
    {
      throw std::runtime_error("Creating RTPSWriterHistory failed");
    }

    writer_ = eprosima::fastrtps::rtps::RTPSDomain::createRTPSWriter(participant_, writer_attr_, history_.get(), nullptr);
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
  static constexpr uint32_t MAX_UNFRAGMENTED_PAYLOAD_SIZE = 64000;  /// Has to be less than UDP packet size

  /// \note: write even if getNumMatchedReaders() == 0, because late subscribers may need historical information

  if (writer_ != nullptr)
  {
    const std::function<uint32_t()> dataSizeFunc = [dataLength]() -> uint32_t { return dataLength * sizeof(uint8_t); };

    eprosima::fastrtps::rtps::CacheChange_t* pChange = writer_->new_change(dataSizeFunc, eprosima::fastrtps::rtps::ChangeKind_t::ALIVE);

    // In the case history is full, remove some old changes
    /// \todo implement a policy for number of old changes to remove
    const uint32_t nMaxChangesToRemove = 1;
    uint32_t nRemoved = 0;
    while (pChange == nullptr)
    {
      /// \note: I am assuming this removes the oldest changes, not the latest ones
      writer_->remove_older_changes(nMaxChangesToRemove);
      ++nRemoved;
      pChange = writer_->new_change(dataSizeFunc, eprosima::fastrtps::rtps::ChangeKind_t::ALIVE);
    }

    // copy data into buffer and send
    pChange->serializedPayload.length = dataSizeFunc();
    if (pChange->serializedPayload.length > MAX_UNFRAGMENTED_PAYLOAD_SIZE)
    {
      pChange->setFragmentSize(MAX_UNFRAGMENTED_PAYLOAD_SIZE);
    }
    memcpy(pChange->serializedPayload.data, pData, dataLength);
    history_->add_change(pChange);
  }
  else
  {
    throw std::runtime_error("Writer not initialised");
  }
}

}
