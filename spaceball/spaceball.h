#include "serial.h"
#include <cstddef>
#include <vector>
#include <iostream>

using SpaceballEvent = std::vector<std::byte>;


std::ostream& operator <<(std::ostream &, const SpaceballEvent &);

class Spaceball : public Serial {
public:
    Spaceball(const char*);    
    SpaceballEvent NextEvent(void);
};