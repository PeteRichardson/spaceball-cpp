#include "spaceball.h"
#include <iostream>
#include <iomanip>

using std::cout, std::endl;



Spaceball::Spaceball(const char* device_path) : Serial(device_path, 9600) {
    // Send initialization cmds to Spaceball

    char spaceballInitString[] = "\rCB\rNT\rFT?\rFR?\rP@r@r\rMSSV\rZ\rBc\r";
    //   "\r"                 # Clear the line
    //   "CB\r"               # Communications Mode Set to Binary
    //   "NT\rFT?\rFR?\r"     # Set Null Region and Trans and Rot Sensitivity
    //   "P@r@r\r"            # Data rate to 20 events per Second
    //   "MSSV\r"             # Ball Event Type: Trans and Rot Vectors
    //   "Z\r"                # Rezero the ball
    //    "Bc\r"              # Beep it

    ssize_t numBytes = write(fileDescriptor, spaceballInitString, strlen(spaceballInitString));   
    if (numBytes != strlen(spaceballInitString))
        printf("Error writing to Spaceball. Wrote %zd bytes - %s(%d).\n", numBytes, strerror(errno), errno);
    // TODO:  this should probably throw instead
}

#define MAX_EVENT_SIZE 32
void Spaceball::Stream() {
    std::byte event[MAX_EVENT_SIZE];
    std::byte *eventPtr = event;
    while(true) {
        auto length = this->getEvent(eventPtr);
        //std::cout << "length = " << length << std::endl;
        this->dumpEvent(eventPtr, length);
    }
}

ssize_t Spaceball::getEvent(std::byte *event) {
    ssize_t numBytes{};       // Number of bytes read or written
    std::byte byte{};
    std::byte *next = event;

    // Find a D or K indicating start of an event
    while (byte != std::byte{'D'} && byte != std::byte{'K'})
        numBytes = read(this->fileDescriptor, &byte, 1);
    *next++ = byte;
    
    // Read until the \r indicating end of event
    while (byte != std::byte{'\r'}) {
        numBytes = read(this->fileDescriptor, &byte, 1);
        *next++ = byte;
    }
    return (next - event - 1);
}

void Spaceball::dumpEvent(const std::byte *event, const ssize_t length) {
    std::cout << char(event[0]);   // event type.  Typically 'K' or 'D'
    for (int i=1; i<length; i++) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << int(event[i]);
    }
    std::cout << std::endl;
}
