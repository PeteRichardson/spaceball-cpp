#include "serial.h"
#include <cstddef>
#include <vector>
#include <iostream>

class SpaceballEvent : public std::vector<std::byte> {
public:
    char type{};
    void Dump();
};

std::ostream& operator <<(std::ostream &, const SpaceballEvent &);

class Spaceball : public Serial {
public:
    Spaceball(const char*);    
    SpaceballEvent NextEvent(void);
};