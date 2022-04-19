#ifndef SERIAL_H
#define SERIAL_H

#include <string>
#include <termios.h>
#include <unistd.h>

class Serial {
public:
    std::string portPath;
    unsigned int baud;
    int fileDescriptor;
    struct termios gOriginalTTYAttrs;

    void connect();
    void disconnect();
    Serial(std::string, unsigned int);
    ~Serial();

    int openSerialPort();
    bool initializeSpaceball();
    ssize_t getEvent(uint8_t *);
    void dumpEvent(const uint8_t *, const ssize_t);
};

#endif