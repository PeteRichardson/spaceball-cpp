#include "spaceball.h"
#include <iostream>
#include <iomanip>

using std::cout, std::endl;

void SpaceballEvent::Dump(void) {
    std::cout << char(this->at(0));   // event type.  Typically 'K' or 'D'
    for (int i=1; i<this->size(); i++) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << int(this->at(i));
    }
    std::cout << std::endl;
}

std::ostream& operator <<(std::ostream & out, const SpaceballEvent &event) {
    out << char(event.at(0));   // event type.  Typically 'K' or 'D'
    out << std::hex;
    for (int i=1; i< event.size(); i++) {
        out << std::setw(2) << std::setfill('0') << int(event.at(i));
    }
    return out;
}

Spaceball::Spaceball(const char* device_path) : Serial(device_path, 9600) {
    // Send initialization cmds to Spaceball
    // See full description in docs/sbprotocol.txt
    char spaceballInitString[] = "\rCB\rNT\rFT?\rFR?\rP@r@r\rMSSV\rZ\rBcAc\r";
    //   "\r"                 # Clear the line
    //   "CB\r"               # Communications Mode Set to Binary
    //   "NT\rFT?\rFR?\r"     # Set Null Region and Trans and Rot Sensitivity
    //   "P@r@r\r"            # Data rate to 20 events per Second
    //   "MSSV\r"             # Ball Event Type: Trans and Rot Vectors
    //   "Z\r"                # Rezero the ball
    //   "BcAc\r"             # Beep twice to indicate completion ('c' = beep, 'A' = pause)

    ssize_t numBytes = write(fileDescriptor, spaceballInitString, strlen(spaceballInitString));   
    if (numBytes != strlen(spaceballInitString))
        throw "Error writing to Spaceball";
}


SpaceballEvent Spaceball::NextEvent(void) {
    auto result = SpaceballEvent{};
    ssize_t numBytes{};       // Number of bytes read or written
    std::byte byte{};

    // Find a D or K indicating start of an event
    while (byte != std::byte{'D'} && byte != std::byte{'K'})
        numBytes = read(this->fileDescriptor, &byte, 1);
    result.push_back(byte);
    
    // Read until the \r indicating end of event
    while (byte != std::byte{'\r'}) {
        numBytes = read(this->fileDescriptor, &byte, 1);
        result.push_back(byte);
    }
    return result;
}
