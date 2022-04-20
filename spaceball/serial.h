#ifndef SERIAL_H
#define SERIAL_H

#include <string>
#include <termios.h>
#include <unistd.h>

class Serial {
public:
    std::string portPath;               // Path for serial port: e.g. "/dev/tty.usbserial-AJ03ACPV"

    Serial(std::string, unsigned int);  // path to port,  baud rate (e.g. 9600)
    ~Serial();

protected:
    int fileDescriptor;                 // Open file descriptor for serial port
                                        // Used in error messages in subclasses, so protected.

private:
    unsigned int baud;                  // Baud rate
    struct termios gOriginalTTYAttrs;   // saved terminal options.  Restored in disconnect()
};

#endif