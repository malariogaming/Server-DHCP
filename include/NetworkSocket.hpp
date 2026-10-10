#pragma once
#include <string>

// Creeaza socket-ul UDP pe portul 67 și il leaga de o interfata specifica
int createDhcpSocket(const std::string& interfaceName);