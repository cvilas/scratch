#pragma once

#include <fastrtps/rtps/attributes/RTPSParticipantAttributes.h>
#include <fastrtps/rtps/attributes/HistoryAttributes.h>
#include <fastrtps/attributes/TopicAttributes.h>
#include <fastrtps/rtps/participant/RTPSParticipant.h>
#include <fastrtps/rtps/participant/RTPSParticipantListener.h>

#include <chrono>
#include <string>

namespace test
{

class Endpoint
{
    class ParticipantListener : public eprosima::fastrtps::rtps::RTPSParticipantListener
    {
    public:
      ParticipantListener(Endpoint& n);
      ~ParticipantListener() noexcept = default;
      void onParticipantDiscovery(eprosima::fastrtps::rtps::RTPSParticipant* pPart, 
                                  eprosima::fastrtps::rtps::ParticipantDiscoveryInfo&& info) final;
    private:
      Endpoint& endpoint_;
    };

public:
    static constexpr std::chrono::seconds DISCOVERY_ANNOUNCE_PERIOD = std::chrono::seconds(10);
public:
    Endpoint(int domain, const std::string& name);
    virtual ~Endpoint();

protected:
    eprosima::fastrtps::rtps::RTPSParticipantAttributes participant_params_ ;
    eprosima::fastrtps::rtps::HistoryAttributes history_attr_;
    eprosima::fastrtps::TopicAttributes topic_attr_;
    eprosima::fastrtps::rtps::RTPSParticipant* participant_;
    ParticipantListener participant_listener_;
};

} // test

