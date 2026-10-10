#pragma once
#include <string>

// Creeaza socket-ul UDP pe portul 67 si il leaga de o interfata specifica
int createDhcpSocket(const std::string& interfaceName);