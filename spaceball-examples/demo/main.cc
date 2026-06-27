#include <iostream>

#include "spaceball.h"

int main(int argc, char*argv[]) {

    auto sb = Spaceball("/dev/cu.usbserial-AJ03ACPV");
    while (auto event = sb.NextEvent())
        std::cout << *event << std::endl;

    return(EXIT_SUCCESS);
}