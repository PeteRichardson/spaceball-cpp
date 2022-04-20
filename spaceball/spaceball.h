#include "serial.h"
#include <cstddef>

class Spaceball : public Serial {
public:
    const char *device_path;

    Spaceball(const char*);
    void Stream(void);
    
    ssize_t getEvent(std::byte *);
    void dumpEvent(const std::byte *, const ssize_t);
};