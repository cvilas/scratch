#ifndef MESSENGER_H
#define MESSENGER_H

#include <dds/domain/ddsdomain.hpp>
#include <dds/pub/Publisher.hpp>
#include <dds/sub/Subscriber.hpp>
#include <memory>

///=====================================================================================================================
/// implements the messenger interface
///=====================================================================================================================
class Messenger
{
public:
    static constexpr int64_t BRINGUP_TIME_MS = 500;
    static constexpr char BUILTIN_TOPIC_CONTROL_INFO[] = "GrapeControlInfo";
    static constexpr char BUILTIN_TOPIC_CONTROL_PROBE[] = "GrapeControlProbe";
    static constexpr char BUILTIN_MESSAGE_CONTROL_PROBE_ALL[] = "GrapeControlProbe_All";

protected:
    /// setup messaging back-end
    Messenger(int domainId);

    Messenger(const Messenger&) = delete;
    Messenger& operator=(Messenger const&) = delete;

    virtual ~Messenger();

    // by default the messenger is disabled. this allows derived classes to set up
    // listeners first
    void enable();

protected:
    std::shared_ptr<dds::domain::DomainParticipant>  _pParticipant;
};


#endif // MESSENGER_H
