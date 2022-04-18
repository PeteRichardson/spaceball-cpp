#include <iostream>

#include "spaceball.h"

using std::cout, std::endl;

int main(int argc, char*argv[]) {
    cout << "Spaceball Demo\n";

    Spaceball sb = Spaceball("/dev/tty.usbserial-AJ03ACPV");
    cout << "version " << sb.getVersion() << endl;

    sb.Stream();

    cout << "Spaceball Demo complete.\n";

}