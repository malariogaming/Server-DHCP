#include <iostream>
#include <unistd.h>
#include "NetworkSocket.hpp"

int main() {
    std::string interface = "ens37";
    std::cout << "Pornire DHCP Server pe interfața " << interface << "..." << std::endl;
    
    int sockfd = createDhcpSocket(interface);
    if (sockfd < 0) {
        std::cerr << "Esec la initializarea serverului. Trebuie rulat cu sudo!" << std::endl;
        return 1;
    }

    std::cout << "Socket deschis cu succes! Gata pentru a primi pachete DHCP." << std::endl;

    // Serverul va ramane pornit 10 secunde pentru test, apoi se va inchide.
    // Ulterior, aici vom pune clasa DhcpServer și bucla de ascultare.
    sleep(10);
    close(sockfd);
    
    return 0;
}