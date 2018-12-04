#include <gsl/span>
#include <cinttypes>
#include <memory>

enum class MessageType
{
  NMTControl = 0x000,  //!< Network management - node control
  Sync = 0x080,        //!< Synchronisation
  Time = 0x0100,       //!< Timestamp
  TPDO1 = 0x0180,      //!< Transmit PDO 1
  RPDO1 = 0x0200,      //!< Receive PDO 1
  TPDO2 = 0x0280,      //!< Transmit PDO 2
  RPDO2 = 0x0300,      //!< Receive PDO 2
  TPDO3 = 0x0380,      //!< Transmit PDO 3
  RPDO3 = 0x0400,      //!< Receive PDO 3
  TPDO4 = 0x0480,      //!< Transmit PDO 4
  RPDO4 = 0x0500,      //!< Receive PDO 4
  TSDO = 0x0580,       //!< Transmit SDO
  RSDO = 0x0600,       //!< Receive SDO
  NMTMonitor = 0x0700  //!< Network management - node monitoring (heartbeat)
};

class Message
{
public:
    virtual MessageType getMessageType() const = 0;
    virtual int8_t getNodeId() const = 0;
    gsl::span<uint8_t>& data();
protected:
    Message(MessageType msg_type, int8_t node_id);
private:
    struct Impl;
    std::unique_ptr<Impl> pimpl;
};

template <MessageType msg_type>
class MessageT : protected Message
{
public:
  MessageT(int8_t node_id) : Message(msg_type, node_id){}
};

struct my_data
{
    int64_t msg;
    void encode(std::unique_ptr<Message>& msg)
    {
    }
};

int main()
{}
