#include <iostream>
#include <unistd.h>
#include <vector>
#include <iomanip>
#include <sys/socket.h>
#include <netinet/in.h>
#include "NetworkSocket.hpp"
#include "DhcpPacket.hpp"

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
                }
                std::cout << "--------------------------------------------------\n";
            }
        }
    }

    close(sockfd);
    return 0;
}