#include <iostream>
#include <unistd.h>
#include <vector>
#include <iomanip>
#include <sys/socket.h>
#include <netinet/in.h>
#include "NetworkSocket.hpp"
#include "DhcpPacket.hpp"
#include <arpa/inet.h>

int main() {
    std::string interface = "ens37";
    std::cout << "Pornire DHCP Server pe interfata " << interface << "..." << std::endl;
    
    int sockfd = createDhcpSocket(interface);
    if (sockfd < 0) {
        std::cerr << "Esec la initializarea serverului. Trebuie rulat cu sudo!" << std::endl;
        return 1;
    }

    std::cout << "Socket deschis cu succes! Gata pentru a primi pachete DHCP." << std::endl;

    while (true) {
        std::vector<uint8_t> buffer(2048); // Buffer mare pentru orice pachet DHCP
        struct sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);

        // Functia blocanta: sta aici pana cand vine un pachet pe portul 67
        ssize_t bytes_received = recvfrom(sockfd, buffer.data(), buffer.size(), 0,
                                        (struct sockaddr*)&client_addr, &client_len);
        
        if (bytes_received > 0) {
            DhcpPacket packet;
            if (packet.parse(buffer.data(), bytes_received)) {
                std::cout << "[+] Am primit un pachet de " << bytes_received << " octeti!" << std::endl;
                
                // Afisam MAC-ul clientului din campul chaddr (Client Hardware Address)
                std::cout << "    MAC Client: ";
                for (int i = 0; i < 6; ++i) {
                    std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)packet.header.chaddr[i];
                    if (i < 5) std::cout << ":";
                }
                std::cout << std::dec << std::endl; // Revenim la format zecimal

                // Optiunea 53 contine DHCP Message Type (1 = Discover, 3 = Request, etc...)
                auto msgType = packet.getOption(53);
                if (!msgType.empty()) {
                    std::cout << "    Tip Mesaj DHCP (Opt 53): " << (int)msgType[0] << std::endl;

                    // Daca este DHCPDISCOVER (1), raspundem cu DHCPOFFER (2)
                    if (msgType[0] == 1) {
                        std::cout << "    => Am detectat DHCPDISCOVER. Construiesc DHCPOFFER..." << std::endl;

                        DhcpPacket offer;
                        offer.header = packet.header; // Pastram xid-ul (Transaction ID) și MAC-ul clientului
                        
                        offer.header.op = 2; // BOOTREPLY (Serverul raspunde)
                        // Oferim clientului IP-ul 192.168.100.10 (momentan nu este o selectie dinamica, e doar de testing)
                        offer.header.yiaddr = inet_addr("192.168.100.10");
                        // Ne identificam pe noi ca server
                        offer.header.siaddr = inet_addr("192.168.100.1");

                        // Adaugam Optiunile DHCP specifice ofertei
                        offer.addOption(53, {2}); // Tip Mesaj: DHCPOFFER
                        offer.addOption(1, {255, 255, 255, 0}); // Subnet Mask
                        offer.addOption(54, {192, 168, 100, 1}); // Server Identifier
                        offer.addOption(51, {0x00, 0x00, 0x0E, 0x10}); // Lease Time: 3600 secunde (1 ora)

                        auto responseBuffer = offer.serialize();

                        // Pregatim adresa de destinatie: Broadcast pe portul Clientului (68)
                        struct sockaddr_in bcast_addr{};
                        bcast_addr.sin_family = AF_INET;
                        bcast_addr.sin_port = htons(68);
                        bcast_addr.sin_addr.s_addr = inet_addr("255.255.255.255");

                        ssize_t sent = sendto(sockfd, responseBuffer.data(), responseBuffer.size(), 0,
                                            (struct sockaddr*)&bcast_addr, sizeof(bcast_addr));
                        
                        if (sent > 0) {
                            std::cout << "    [+] DHCPOFFER trimis cu succes (" << sent << " octeti)!" << std::endl;
                        } else {
                            perror("    [-] Eroare la trimiterea DHCPOFFER");
                        }
                    }
                    else if (msgType[0] == 3) {
                        std::cout << "    => Am detectat DHCPREQUEST. Construiesc DHCPACK..." << std::endl;

                        DhcpPacket ack;
                        ack.header = packet.header; // Pastram xid-ul și MAC-ul clientului
                        
                        ack.header.op = 2; // BOOTREPLY
                        // Confirmam adresa IP alocata
                        ack.header.yiaddr = inet_addr("192.168.100.10"); // nu este o selectie dinamica, este doar de testing
                        ack.header.siaddr = inet_addr("192.168.100.1");

                        // Adaugam Opțiunile DHCP specifice confirmarii
                        ack.addOption(53, {5}); // Tip Mesaj: DHCPACK (5)
                        ack.addOption(1, {255, 255, 255, 0}); // Subnet Mask
                        ack.addOption(54, {192, 168, 100, 1}); // Server Identifier
                        ack.addOption(51, {0x00, 0x00, 0x0E, 0x10}); // Lease Time: 3600 secunde
                        
                        // Optional: Putem oferi acum si Default Gateway (Opt 3) și un server DNS (Opt 6)
                        ack.addOption(3, {192, 168, 100, 1}); // Router / Gateway
                        ack.addOption(6, {8, 8, 8, 8}); // DNS (aceste este DNS-ul de la Google)

                        auto responseBuffer = ack.serialize();

                        // Destinatia este tot Broadcast pe portul 68
                        struct sockaddr_in bcast_addr{};
                        bcast_addr.sin_family = AF_INET;
                        bcast_addr.sin_port = htons(68);
                        bcast_addr.sin_addr.s_addr = inet_addr("255.255.255.255");

                        ssize_t sent = sendto(sockfd, responseBuffer.data(), responseBuffer.size(), 0,
                                            (struct sockaddr*)&bcast_addr, sizeof(bcast_addr));
                        
                        if (sent > 0) {
                            std::cout << "    [+] DHCPACK trimis cu succes (" << sent << " octeti)!" << std::endl;
                        } else {
                            perror("    [-] Eroare la trimiterea DHCPACK");
                        }
                    }
                }
                std::cout << "--------------------------------------------------\n";
            }
        }
    }

    close(sockfd);
    return 0;
}