#include <iostream>
#include "serial.h"

#define MAX_EVENT_SIZE 32

int main(int argc, char*argv[]) {
    std::cout << "Hello, serial_connect!" << std::endl;
    
    auto ser = Serial("/dev/tty.usbserial-AJ03ACPV", 9600);
    ser.connect();

    uint8_t event[MAX_EVENT_SIZE];
    uint8_t *eventPtr = event;
    while(true) {
        auto length = ser.getEvent(eventPtr);
        //std::cout << "length = " << length << std::endl;
        ser.dumpEvent(event, length);
    }

    ser.disconnect();
    std::cout << "Goodbye, serial_connect!" << std::endl;
}