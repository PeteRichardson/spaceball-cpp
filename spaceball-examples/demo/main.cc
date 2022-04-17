#include <iostream>

#include "spaceball.h"

using std::cout, std::endl;

int main(int argc, char*argv[]) {
    Spaceball sb = Spaceball();

    cout << "Spaceball Demo\n";
    cout << "version " << sb.getVersion() << endl;
}