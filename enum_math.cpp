#include <cinttypes>
#include <iostream>
#include <type_traits>

enum class MessageID : uint16_t
{
    Nmt = 0x000,
    Sync = 0x080,
    Timestamp = 0x0100,
    TransmitPDO1 = 0x0180,
    ReceivePDO1 = 0x0200,
    TransmitPDO2 = 0x0280,
    ReceivePDO2 = 0x0300,
    TransmitPDO3 = 0x0380,
    ReceivePDO3 = 0x0400,
    TransmitPDO4 = 0x0480,
    ReceivePDO4 = 0x0500,
    TransmitSDO = 0x0580,
    ReceiveSDO = 0x0600,
    Heartbeat = 0x0700    
    
};

int main()
{
    MessageID cob;
    uint8_t device = 0x01;
    std::cout << std::hex << static_cast<std::underlying_type<MessageID>::type>(MessageID::TransmitPDO1) + device << std::endl;
    return 0;
}
