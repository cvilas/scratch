#include <fastrtps/fastrtps_all.h>
#include <fastrtps/rtps/reader/RTPSReader.h>
#include <fastrtps/rtps/reader/ReaderListener.h>
#include <fastrtps/rtps/reader/StatefulReader.h>
#include <fastrtps/Domain.h>
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/rtps/builtin/discovery/endpoint/EDPSimple.h>

using namespace eprosima::fastrtps;
using namespace eprosima::fastcdr;

class CustomParticipantListener : public eprosima::fastrtps::ParticipantListener
{
  struct part_info
  {
    GuidPrefix_t guidpre;
    std::string name;
  };

  std::vector<part_info> part_names;
  void onParticipantDiscovery(Participant* participant, rtps::ParticipantDiscoveryInfo&& info) override
  {
    part_info p;
    p.name = info.info.m_participantName;
    p.guidpre = info.info.m_guid.guidPrefix;
    part_names.push_back(p);
  }

/* Custom Listener onSubscriberDiscovery */
void onSubscriberDiscovery(eprosima::fastrtps::Participant * participant, eprosima::fastrtps::rtps::ReaderDiscoveryInfo && info) override
{
  std::string pname;
  auto pre = info.info.guid().guidPrefix;
  auto it = std::find_if(part_names.begin(), part_names.end(), [&pre](const part_info& p){return p.guidpre == pre;});
  if(it != part_names.end())
  {
    pname = it->name;
  }

  switch(info.status)
  {
    case eprosima::fastrtps::rtps::ReaderDiscoveryInfo::DISCOVERED_READER:
      /* Process the case when a new subscriber was found in the domain */
      cout << pname << "\tSubscriber - topic '" << info.info.topicName() << "'of type '" << info.info.typeName() << "' discovered\n";
      break;
    case eprosima::fastrtps::rtps::ReaderDiscoveryInfo::CHANGED_QOS_READER:
      /* Process the case when a subscriber changed its QOS */
      break;
    case eprosima::fastrtps::rtps::ReaderDiscoveryInfo::REMOVED_READER:
      /* Process the case when a subscriber was removed from the domain */
      cout << pname << "\tSubscriber - topic '" << info.info.topicName() << "' of type '" << info.info.typeName() << "' left the domain.\n";
      break;
  }
}

/* Custom Listener onPublisherDiscovery */
void onPublisherDiscovery(eprosima::fastrtps::Participant * participant, eprosima::fastrtps::rtps::WriterDiscoveryInfo && info) override
{
  std::string pname;
  auto pre = info.info.guid().guidPrefix;
  auto it = std::find_if(part_names.begin(), part_names.end(), [&pre](const part_info& p){return p.guidpre == pre;});
  if(it != part_names.end())
  {
    pname = it->name;
  }
  switch(info.status)
  {
    case eprosima::fastrtps::rtps::WriterDiscoveryInfo ::DISCOVERED_WRITER:
      /* Process the case when a new publisher was found in the domain */
      cout << pname << "\tPublisher  - topic '" << info.info.topicName() << "'of type '" << info.info.typeName() << "' discovered\n";
      break;
    case eprosima::fastrtps::rtps::WriterDiscoveryInfo ::CHANGED_QOS_WRITER:
      /* Process the case when a publisher changed its QOS */
      break;
    case eprosima::fastrtps::rtps::WriterDiscoveryInfo ::REMOVED_WRITER:
      /* Process the case when a publisher was removed from the domain */
      cout << pname << "\tPublisher - topic '" << info.info.topicName() << "' of type '" << info.info.typeName() << "' left the domain.\n";
      break;
  }
}
};

int main()
{
  // Create Custom user ParticipantListener (should inherit from eprosima::fastrtps::ParticipantListener.
  CustomParticipantListener *listener = new CustomParticipantListener();

  // Pass the listener on participant creation.
  ParticipantAttributes pattr;
  pattr.rtps.setName("PROBER");
  pattr.rtps.builtin.domainId = 1;

  Participant* participant = Domain::createParticipant(pattr, listener);

  while(1)
  {
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
}
