#include "serial.h"
#include <cstddef>
#include <optional>
#include <vector>
#include <iostream>

using SpaceballEvent = std::vector<std::byte>;


std::ostream& operator <<(std::ostream &, const SpaceballEvent &);

class Spaceball : public Serial {
public:
    Spaceball(const char*);
    std::optional<SpaceballEvent> NextEvent(void);
};