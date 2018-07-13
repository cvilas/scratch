#include "MonitorMessenger.h"

#include <dds/topic/Topic.hpp>
#include <dds/sub/find.hpp>
#include <rti/core/ListenerBinder.hpp>

#include <iostream>
#include <cstddef>

//=====================================================================================================================
class ControlInfoListener : public dds::sub::NoOpDataReaderListener<dds::core::KeyedBytesTopicType>
//=====================================================================================================================
{
public:
    ControlInfoListener(MonitorMessenger& messenger)
        : _messenger(messenger) {}

    ~ControlInfoListener()
    {
        std::cout << "[~ControlInfoListener]" << std::endl;
    }

    void on_data_available(dds::sub::DataReader<dds::core::KeyedBytesTopicType>& reader) final
    {
        dds::sub::LoanedSamples<dds::core::KeyedBytesTopicType> samples = reader.take();
        for (const auto& sample : samples)
        {
            if (sample.info().valid())
            {
                const dds::core::KeyedBytesTopicType& data = static_cast<const dds::core::KeyedBytesTopicType&>(sample.data());

                std::string key = data.key().to_std_string();
                const std::vector<uint8_t> value = data.value();

                //std::cout << "[ControlInfoListener] received info for: " << key << std::endl;

                std::lock_guard<std::mutex> guard(_messenger._controlMutex);
                const auto& it = _messenger._controlMonitors.find(key);
                if( it != _messenger._controlMonitors.end() )
                {
                    AbstractControlMonitor* pMonitor = _messenger._controlMonitors.at(key).get();
                    pMonitor->callback(value);
                }
            }
        }
    }
private:
    MonitorMessenger& _messenger;
};

//---------------------------------------------------------------------------------------------------------------------
void MonitorMessenger::probeControlInfo(const std::string& topicName)
//---------------------------------------------------------------------------------------------------------------------
{
    //std::cout << "[MonitorMessenger::probeControlInfo] Probe: " << topicName << std::endl;
    _controlProbeWriter.write(topicName);
}

//---------------------------------------------------------------------------------------------------------------------
void MonitorMessenger::probeAllControlInfo()
//---------------------------------------------------------------------------------------------------------------------
{
    //std::cout << "[MonitorMessenger::probeAllControlInfo]" << std::endl;
    _controlProbeWriter.write(Messenger::BUILTIN_MESSAGE_CONTROL_PROBE_ALL);
}

//=====================================================================================================================
class PublicationBuiltinTopicsListener : public dds::sub::NoOpDataReaderListener<dds::topic::PublicationBuiltinTopicData>
//=====================================================================================================================
{
public:
    PublicationBuiltinTopicsListener(std::vector<std::string>& topics)
        : _topics(topics) {}

    ~PublicationBuiltinTopicsListener()
    {
        std::cout << "[~PublicationBuiltinTopicsListener]" << std::endl;
    }

    // This gets called when a publisher has been discovered
    void on_data_available(dds::sub::DataReader<dds::topic::PublicationBuiltinTopicData>& reader)
    {
        // We only process newly seen subscribers
        dds::sub::LoanedSamples<dds::topic::PublicationBuiltinTopicData> samples = reader.select().state(dds::sub::status::DataState::new_instance()).take();

        for (const auto& sample : samples)
        {
            if( !sample.info().valid() )
            {
                continue;
            }

            const dds::topic::PublicationBuiltinTopicData& data = sample.data();
            std::string topicName = data.topic_name();

            // skip the builtin topic for control probing
            if( topicName == Messenger::BUILTIN_TOPIC_CONTROL_INFO )
            {
                continue;
            }

            ///@todo
            /// - how do we detect unregistration of a log variable at the plant?

            _topics.push_back(topicName);
        }
    }

private:
    std::vector<std::string>& _topics;
};

//=====================================================================================================================
class SubscriptionBuiltinTopicsListener : public dds::sub::NoOpDataReaderListener<dds::topic::SubscriptionBuiltinTopicData>
//=====================================================================================================================
{
public:
    SubscriptionBuiltinTopicsListener(std::vector<std::string>& topics)
        : _topics(topics)
    {}

    ~SubscriptionBuiltinTopicsListener()
    {
        std::cout << "[~SubscriptionBuiltinTopicsListener]" << std::endl;
    }

    // This gets called when a subscriber has been discovered
    void on_data_available(dds::sub::DataReader<dds::topic::SubscriptionBuiltinTopicData>& reader)
    {
         // We only process newly seen subscribers
        dds::sub::LoanedSamples<dds::topic::SubscriptionBuiltinTopicData> samples = reader.select().state(dds::sub::status::DataState::new_instance()).take();

        for (const auto& sample : samples)
        {
            if( !sample.info().valid() )
            {
                continue;
            }

            const dds::topic::SubscriptionBuiltinTopicData& data = sample.data();
            std::string topicName = data.topic_name();

            // skip the builtin topic for control probing
            if( topicName == Messenger::BUILTIN_TOPIC_CONTROL_PROBE )
            {
                continue;
            }

            ///@todo
            /// - how do we detect unregistration of a control variable at the plant?

            _topics.push_back(topicName);
        }
    }
private:
    std::vector<std::string>& _topics;
};

//=====================================================================================================================
MonitorMessenger::MonitorMessenger(int domainId)
//=====================================================================================================================
    : Messenger(domainId),
      _publisher(*_pParticipant),
      _subscriber(*_pParticipant),
      _controlProbeWriter(_publisher, dds::topic::Topic<dds::core::StringTopicType>(*_pParticipant, Messenger::BUILTIN_TOPIC_CONTROL_PROBE)),
      _controlInfoReader(_subscriber, dds::topic::Topic<dds::core::KeyedBytesTopicType>(*_pParticipant, Messenger::BUILTIN_TOPIC_CONTROL_INFO))
{
    // Get the builtin subscriber and then the data reader to determine what topics are active
    // on all participants other than this class
    dds::sub::Subscriber builtinSubscriber = dds::sub::builtin_subscriber(*_pParticipant);

    // Get builtin subscriber's datareader for publications.
    std::vector< dds::sub::DataReader<dds::topic::PublicationBuiltinTopicData> > publicationReaders;
    dds::sub::find< dds::sub::DataReader<dds::topic::PublicationBuiltinTopicData> >(builtinSubscriber,
                                                                                    dds::topic::publication_topic_name(),
                                                                                    std::back_inserter(publicationReaders));

    // Get builtin subscriber's datareader for subscriptions.
    std::vector< dds::sub::DataReader<dds::topic::SubscriptionBuiltinTopicData> > subscriptionReaders;
    dds::sub::find< dds::sub::DataReader<dds::topic::SubscriptionBuiltinTopicData> >(builtinSubscriber,
                                                                                    dds::topic::subscription_topic_name(),
                                                                                    std::back_inserter(subscriptionReaders));

    // Making the following static ensures that the listeners and readers live as long as the
    // messenger lives. When the messenger dies, the listener binder ensures that the listener is reset on the
    // corresponding reader, and the listener and reader are deleted
    //@note: alternative is to call DataReader::listener on an existing listener

    // Install a listener to collect published topics (log topics)
    static rti::core::ListenerBinder< dds::sub::DataReader<dds::topic::PublicationBuiltinTopicData> >
            publicationsListener = rti::core::bind_and_manage_listener(publicationReaders[0],
                new PublicationBuiltinTopicsListener(_logTopics),
                dds::core::status::StatusMask::data_available());

    // Install a listener to collect subscribed topics (control topics)
    static rti::core::ListenerBinder< dds::sub::DataReader<dds::topic::SubscriptionBuiltinTopicData> >
            subscriptionsListener = rti::core::bind_and_manage_listener(subscriptionReaders[0],
                new SubscriptionBuiltinTopicsListener(_controlTopics),
                dds::core::status::StatusMask::data_available());

    // Install a listener for control variables update
    static rti::core::ListenerBinder<dds::sub::DataReader<dds::core::KeyedBytesTopicType> >
            controlInfoListener = rti::core::bind_and_manage_listener(_controlInfoReader,
                                                                      new ControlInfoListener(*this),
                                                                      dds::core::status::StatusMask::data_available());

    // now enable the participant and start receiving messages
    enable();

    // probe all control variables at the plant
    probeAllControlInfo();
}

//---------------------------------------------------------------------------------------------------------------------
MonitorMessenger::~MonitorMessenger()
//---------------------------------------------------------------------------------------------------------------------
{}

//---------------------------------------------------------------------------------------------------------------------
void MonitorMessenger::unmonitorLogVariable(const std::string& name)
//---------------------------------------------------------------------------------------------------------------------
{
    _logMonitors.erase(name);
}

//---------------------------------------------------------------------------------------------------------------------
void MonitorMessenger::unmonitorControlVariable(const std::string& name)
//---------------------------------------------------------------------------------------------------------------------
{
    std::lock_guard<std::mutex> guard(_controlMutex);
    _controlMonitors.erase(name);
}

