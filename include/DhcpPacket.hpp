#pragma once
#include <cstdint>
#include <vector>
#include <string>

// Previne padding de catre compilator
#pragma pack(push, 1)
struct DhcpHeader {
    uint8_t  op;              // 1 = BOOTREQUEST, 2 = BOOTREPLY
    uint8_t  htype;           // 1 = Ethernet (10Mb)
    uint8_t  hlen;            // 6 (lungimea adresei MAC)
    uint8_t  hops;            // 0
    uint32_t xid;             // Transaction ID
    uint16_t secs;            // Secunde trecute
    uint16_t flags;           // Broadcast flag
    uint32_t ciaddr;          // Client IP address (daca este reinnoit)
    uint32_t yiaddr;          // 'Your' (client) IP address (alocat de server)
    uint32_t siaddr;          // Next server IP
    uint32_t giaddr;          // Relay agent IP
    uint8_t  chaddr[16];      // Client hardware address (MAC-ul ocupa primii 6 octeti)
    char     sname[64];       // Server host name (opțional)
    char     file[128];       // Boot file name (opțional)
    uint32_t magic_cookie;    // 0x63825363 (standard DHCP)
};
#pragma pack(pop)

class DhcpPacket {
public:
    DhcpHeader header{};
    std::vector<uint8_t> options;

    bool parse(const uint8_t* buffer, size_t length);
    std::vector<uint8_t> serialize() const;
    void addOption(uint8_t code, const std::vector<uint8_t>& data);
    std::vector<uint8_t> getOption(uint8_t code) const;
};