#include <iostream>

#include "spaceball.h"

int main(int argc, char*argv[]) {
    std::cout << "Spaceball Demo\n";

    auto sb = Spaceball("/dev/tty.usbserial-AJ03ACPV");
    sb.connect();

    sb.Stream();

    std::cout << "Spaceball Demo complete." << std::endl;
}