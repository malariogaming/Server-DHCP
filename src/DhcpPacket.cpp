#include "DhcpPacket.hpp"
#include <cstring>
#include <iostream>

bool DhcpPacket::parse(const uint8_t* buffer, size_t length) {
    // Verificam dacă bufferul este suficient de mare macar pentru headerul fix
    if (length < sizeof(DhcpHeader)) {
        return false; 
    }

    // Copiem octetii direct în structura noastra din memorie
    std::memcpy(&header, buffer, sizeof(DhcpHeader));

    // Calculam cat au mai ramas pentru optiuni si le copiem în vector
    size_t options_len = length - sizeof(DhcpHeader);
    if (options_len > 0) {
        options.assign(buffer + sizeof(DhcpHeader), buffer + length);
    } else {
        options.clear();
    }

    return true;
}

std::vector<uint8_t> DhcpPacket::getOption(uint8_t code) const {
    size_t i = 0;
    // Parcurgem vectorul de optiuni în format TLV (Type-Length-Value)
    while (i < options.size()) {
        uint8_t opt_code = options[i];
        
        if (opt_code == 255) break; // Codul 255 inseamnă sfarsitul optiunilor
        if (opt_code == 0) {        // Codul 0 este padding (se ignoră)
            i++;
            continue;
        }
        
        if (i + 1 >= options.size()) break; // Pachet malformat (lipseste lungimea)
        uint8_t opt_len = options[i + 1];
        
        if (opt_code == code) {
            // Daca am gasit codul cerut, extragem datele
            if (i + 2 + opt_len <= options.size()) {
                return std::vector<uint8_t>(options.begin() + i + 2, options.begin() + i + 2 + opt_len);
            }
            break; // Pachet malformat (lungimea depaseete bufferul)
        }
        
        // Sarim la urmatoarea optiune: 1 (cod) + 1 (lungime) + lungime date
        i += 2 + opt_len;
    }
    return {}; // Optiunea nu a fost gasita
}

// Lasam aceste doua functii goale momentan, le vom scrie cand implementam raspunsul (DHCPOFFER)
std::vector<uint8_t> DhcpPacket::serialize() const { return {}; }
void DhcpPacket::addOption(uint8_t code, const std::vector<uint8_t>& data) {}