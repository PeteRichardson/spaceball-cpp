#include "spaceball.h"
#include <iostream>
#include <iomanip>

#define KEYP_MASK 0x1000
#define KEY1_MASK 0x0001
#define KEY2_MASK 0x0002
#define KEY3_MASK 0x0004
#define KEY4_MASK 0x0008
#define KEY5_MASK 0x0100
#define KEY6_MASK 0x0200
#define KEY7_MASK 0x0400
#define KEY8_MASK 0x0800

// def twos_comp(val, bits):
//     """compute the 2's compliment of int value val"""
//     if (val & (1 << (bits - 1))) != 0: # if sign bit is set e.g., 8bit: 128-255
//         val = val - (1 << bits)        # compute negative value
//     return val                         # return positive value as is


std::ostream& operator <<(std::ostream & out, const SpaceballEvent &event) {
    char type = char(event[0]);
    if (type == 'K') {
        // Anytime a key is pressed or released a "K" packet is sent from the Spaceball 
        // that indicates the new state of the nine buttons (eight buttons labelled 1-8 
        // and the pick button which is under the skin on the far side of the ball).  In
        // the two bytes between the "K" and the <cr> a 1 bit indicates the button is 
        // pressed, a 0 bit indicates a button is not pressed. The packet is laid out
        // as follows:
        // K  010<pick button state><b8><b7><b6><b5>  0100<b4><b3><b2><b1>  \015
        out << type << ": ";       // event type.  Typically 'K' or 'D'
        uint16_t keyflags = (int(event[1]) << 8)  + int(event[2]);
        out << ((keyflags & KEYP_MASK) ? 'P' : '_');
        out << ((keyflags & KEY1_MASK) ? '1' : '_');
        out << ((keyflags & KEY2_MASK) ? '2' : '_');
        out << ((keyflags & KEY3_MASK) ? '3' : '_');
        out << ((keyflags & KEY4_MASK) ? '4' : '_');
        out << ((keyflags & KEY5_MASK) ? '5' : '_');
        out << ((keyflags & KEY6_MASK) ? '6' : '_');
        out << ((keyflags & KEY7_MASK) ? '7' : '_');
        out << ((keyflags & KEY8_MASK) ? '8' : '_');
    } else if (type == 'D') {
        // The bytes between the "D" and the <cr> contain the Period followed by the
        // 6 DOF data.  The Period is an unsigned 16 bit number indicating the amount of time,
        // in 1/16ths of a millisecond, since Spaceball last sent a "D" packet.  The Period is
        // intended for integration with the 6 DOF data that follows.  The 6 DOF data is returned
        // in signed 16 bit 2's complement integers, translation vector first, then rotation
        // vector.  The packet is laid out as follows:
        //     D <period> <delta translation vector> <delta rotation vector> \015
        if (event.size() != 15)
            // out << "Unexpected size. Expected 15.  Found " << event.size() << "\n";
            return out;
        out << type << ": ";       // event type.  Typically 'K' or 'D'
        float period = uint16_t((event[1] << 8) | event[2]) / 16.0;
        out << "per=" << std::setprecision(4) << std::setw(6) << std::dec << period << "ms";
        out << "  T(";
        for (int i = 0; i < 3; i++) {
            out << std::setw(6) << int16_t(int16_t(event[2 * i + 3]) << 8 | int16_t(event[2 * i + 2]));
            if (i < 2)
              out << ",";
        }
        out << ")   R(";
        for (int i = 3; i < 6; i++) {
            out << std::setw(6) << int16_t(int16_t(event[2 * i + 3]) << 8 | int16_t(event[2 * i + 2]));
            if (i < 5)
              out << ",";
        }
        out << ")";
    } else {
        out << type << ": ";       // event type.  Typically 'K' or 'D'
        out << std::hex;
        for (int i=1; i< event.size(); i++)
            out << std::setw(2) << std::setfill('0') << int(event[i]);
        out << std::dec;
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
    auto event = SpaceballEvent{};
    ssize_t numBytes{};       // Number of bytes read or written
    std::byte byte{};

    // Find a D or K indicating start of an event
    while (byte != std::byte{'D'} && byte != std::byte{'K'})
        numBytes = read(this->fileDescriptor, &byte, 1);
    event.push_back(byte);
    
    // Read until the \r indicating end of event
    while (true) {
        numBytes = read(this->fileDescriptor, &byte, 1);
        if (byte == std::byte{'\r'})
            break;
        event.push_back(byte);
    }
    return event;
}
