#include "reader.h"

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
  std::cout << "[Reader::Listener::onReaderMatched]:\n";
  switch (info.status)
  {
    case eprosima::fastrtps::rtps::MatchingStatus::MATCHED_MATCHING:  // new publisher/subscriber found
      std::cout << "\tstatus: MATCHED_MATCHING\n";
      break;
    case eprosima::fastrtps::rtps::MatchingStatus::REMOVED_MATCHING:  // publisher/subscriber removed
      std::cout << "\tstatus: REMOVED_MATCHING\n";
      break;
  };
}

//---------------------------------------------------------------------------------------------------------------------
void Reader::Listener::onNewCacheChangeAdded(eprosima::fastrtps::rtps::RTPSReader* pReader, const eprosima::fastrtps::rtps::CacheChange_t* const pChange)
//---------------------------------------------------------------------------------------------------------------------
{
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
    reader_attr_.endpoint.reliabilityKind = eprosima::fastrtps::rtps::ReliabilityKind_t::RELIABLE;
    reader_attr_.endpoint.durabilityKind = eprosima::fastrtps::rtps::VOLATILE;

    reader_qos_.m_reliability.kind = eprosima::fastrtps::RELIABLE_RELIABILITY_QOS;
    reader_qos_.m_durability.kind = eprosima::fastrtps::VOLATILE_DURABILITY_QOS;

    reader_qos_.m_liveliness.kind = eprosima::fastrtps::AUTOMATIC_LIVELINESS_QOS;
    reader_qos_.m_liveliness.announcement_period.seconds = Endpoint::READER_DISCOVERY_ANNOUNCE_PERIOD.count();
    reader_qos_.m_liveliness.lease_duration.seconds = 10 * reader_qos_.m_liveliness.announcement_period.seconds;

    history_ = std::make_unique<eprosima::fastrtps::rtps::ReaderHistory>(history_attr_);
    if (history_ == nullptr)
    {
      throw std::runtime_error("Creating ReaderHistory failed");
    }
/*
    reader_ = eprosima::fastrtps::rtps::RTPSDomain::createRTPSReader(participant_, reader_attr_, history_.get(), &reader_listener_);
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
*/
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
