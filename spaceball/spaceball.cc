#include "spaceball.h"
#include <iostream>
#include <thread>      // for sleep_for
#include "SerialPort/SerialPort.hpp"

using std::cout, std::endl;

const unsigned int Spaceball::getVersion() {
    return version;
}

Spaceball::Spaceball(const char* device_path) {
    this->device_path = device_path;
    std::cerr << "# Opening device " << device_path << endl;

    const int baudRate = 9600;
    int sfd = openAndConfigureSerialPort(device_path, baudRate);
    if (sfd < 0) {
        if (sfd == -1) {
            std::cerr << "# Boo! Unable to connect to serial port.\n";
        }
        else { //sfd == -2
            std::cerr << "# Error setting serial port attributes.\n";
        }
    }
    if (serialPortIsOpen()) {
        std::cerr << "# Serial port is open." << endl;
    }
}

Spaceball::~Spaceball() {
    auto result = closeSerialPort();
    std::cerr << "# Closing device " << this->device_path << endl;
}

void Spaceball::Stream() {
    std::cerr << "# Starting streaming " << endl;

    std::this_thread::sleep_for(std::chrono::seconds{3});

    // * Read using readSerialData(char* bytes, size_t length)
    
    // * Write using writeSerialData(const char* bytes, size_t length)
    
    // * Remember to flush potentially buffered data when necessary
    flushSerialData();
    std::cerr << "# Done streaming." << endl;

}