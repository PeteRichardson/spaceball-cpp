#include "serial.h"

#include <iostream>
#include <string>
using std::cout, std::endl;

Serial::Serial(std::string portPath, unsigned int baud) :
    portPath{portPath}, baud{baud} {
    cout << "# Constructing Serial(" << portPath << ")" << endl;

}

void Serial::connect() {
    cout << "# Connecting to " << this->portPath << endl;
}

void Serial::disconnect() {
    cout << "# Disconnecting from " << this->portPath << endl;
}

Serial::~Serial() {
    cout << "# Destructing Serial(" << this->portPath << ")" << endl;

}