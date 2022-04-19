#include "serial.h"

class Spaceball : public Serial {
public:
    const char *device_path;

    Spaceball(const char*);
    void Stream(void);
    
    bool initializeSpaceball();
    ssize_t getEvent(uint8_t *);
    void dumpEvent(const uint8_t *, const ssize_t);

};