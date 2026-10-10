#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <unistd.h>

int createDhcpSocket(const std::string& interfaceName) {
    int sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockfd < 0) {
        perror("Eroare la crearea socket-ului");
        return -1;
    }

    // Permite reutilizarea portului pentru a preveni erori la restartarea serverului
    int reuse = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    // Activeaza permisiunea de a trimite si primi pachete Broadcast
    int broadcast = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast)) < 0) {
        perror("Eroare SO_BROADCAST");
        close(sockfd);
        return -1;
    }

    // Leaga socket-ul specific de interfata dedicata testarii (ex. ens37)
    struct ifreq ifr{};
    std::strncpy(ifr.ifr_name, interfaceName.c_str(), IFNAMSIZ - 1);
    if (setsockopt(sockfd, SOL_SOCKET, SO_BINDTODEVICE, &ifr, sizeof(ifr)) < 0) {
        perror("Eroare SO_BINDTODEVICE (necesita rulare cu sudo)");
        close(sockfd);
        return -1;
    }

    // Asculta pe adresa 0.0.0.0 și portul standard 67
    struct sockaddr_in srvAddr{};
    srvAddr.sin_family = AF_INET;
    srvAddr.sin_port = htons(67);
    srvAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd, reinterpret_cast<struct sockaddr*>(&srvAddr), sizeof(srvAddr)) < 0) {
        perror("Eroare la bind pe portul 67");
        close(sockfd);
        return -1;
    }

    return sockfd;
}