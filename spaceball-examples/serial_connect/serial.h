#ifndef SERIAL_H
#define SERIAL_H

#include <string>

class Serial {
public:
    std::string portPath;
    unsigned int baud;

    void connect();
    void disconnect();
    Serial(std::string, unsigned int);
    ~Serial();
};

#endif