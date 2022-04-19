#include <iostream>
#include "serial.h"

int main(int argc, char*argv[]) {
    std::cout << "Hello, serial_connect!" << std::endl;
    
    auto ser = Serial("/dev/tty.usbserial-AJ03ACPV", 9600);
    ser.connect();


    ser.disconnect();
    std::cout << "Goodbye, serial_connect!" << std::endl;
}