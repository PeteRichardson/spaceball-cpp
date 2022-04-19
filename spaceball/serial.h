#ifndef SERIAL_H
#define SERIAL_H

#include <string>
#include <termios.h>
#include <unistd.h>

class Serial {
public:
    std::string portPath;               // Path for serial port: e.g. "/dev/tty.usbserial-AJ03ACPV"

    Serial(std::string, unsigned int);
    ~Serial();

    void connect();

protected:
    unsigned int baud;                  // Baud rate
    int fileDescriptor;                 // Open file descriptor for serial port
    struct termios gOriginalTTYAttrs;   // saved terminal options.  Restored in disconnect()
};

#endif