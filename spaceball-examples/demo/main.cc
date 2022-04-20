#include <iostream>

#include "spaceball.h"

int main(int argc, char*argv[]) {

    auto sb = Spaceball("/dev/tty.usbserial-AJ03ACPV");
    while(true)
        std::cout << sb.NextEvent() << std::endl;

    return(EXIT_SUCCESS);
}