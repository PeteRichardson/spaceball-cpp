#include "spaceball.h"
#include <cstring>
#include <iostream>
#include <iomanip>
#include <stdexcept>
#include <vector>

static constexpr uint16_t KEYP_MASK = 0x1000;
static constexpr uint16_t KEY1_MASK = 0x0001;
static constexpr uint16_t KEY2_MASK = 0x0002;
static constexpr uint16_t KEY3_MASK = 0x0004;
static constexpr uint16_t KEY4_MASK = 0x0008;
static constexpr uint16_t KEY5_MASK = 0x0100;
static constexpr uint16_t KEY6_MASK = 0x0200;
static constexpr uint16_t KEY7_MASK = 0x0400;
static constexpr uint16_t KEY8_MASK = 0x0800;


std::ostream& operator<<(std::ostream& out, const KeyEvent& e) {
    // K packet: 010<pick><b8><b7><b6><b5>  0100<b4><b3><b2><b1>
    out << "K: ";
    out << (e.pick ? 'P' : '_');
    for (int i = 0; i < 8; i++)
        out << (e.buttons[i] ? char('1' + i) : '_');
    return out;
}

std::ostream& operator<<(std::ostream& out, const MotionEvent& e) {
    // D packet: D <period(2)> <Tx(2)> <Ty(2)> <Tz(2)> <Rx(2)> <Ry(2)> <Rz(2)> \r
    out << "D: ";
    out << "per=" << std::setprecision(4) << std::setw(6) << std::dec << e.period << "ms";
    out << "  T(";
    for (int i = 0; i < 3; i++) {
        out << std::setw(6) << e.translation[i];
        if (i < 2) out << ",";
    }
    out << ")   R(";
    for (int i = 0; i < 3; i++) {
        out << std::setw(6) << e.rotation[i];
        if (i < 2) out << ",";
    }
    out << ")";
    return out;
}

std::ostream& operator<<(std::ostream& out, const SpaceballEvent& event) {
    std::visit([&out](const auto& e) { out << e; }, event);
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
    if (numBytes != (ssize_t)strlen(spaceballInitString))
        throw std::runtime_error("Error writing init string to Spaceball");
}


std::optional<SpaceballEvent> Spaceball::NextEvent(void) {
    std::vector<std::byte> raw;
    ssize_t numBytes{};
    std::byte byte{};

    // Find a D or K indicating start of an event
    while (byte != std::byte{'D'} && byte != std::byte{'K'}) {
        numBytes = read(this->fileDescriptor, &byte, 1);
        if (numBytes < 0)
            return std::nullopt;
    }
    raw.push_back(byte);

    // Read until the \r indicating end of event
    while (true) {
        numBytes = read(this->fileDescriptor, &byte, 1);
        if (numBytes < 0)
            return std::nullopt;
        if (byte == std::byte{'\r'})
            break;
        raw.push_back(byte);
    }

    if (char(raw[0]) == 'K') {
        uint16_t keyflags = uint16_t((int(raw[1]) << 8) | int(raw[2]));
        return KeyEvent{
            .pick    = bool(keyflags & KEYP_MASK),
            .buttons = {
                bool(keyflags & KEY1_MASK),
                bool(keyflags & KEY2_MASK),
                bool(keyflags & KEY3_MASK),
                bool(keyflags & KEY4_MASK),
                bool(keyflags & KEY5_MASK),
                bool(keyflags & KEY6_MASK),
                bool(keyflags & KEY7_MASK),
                bool(keyflags & KEY8_MASK),
            }
        };
    } else { // 'D'
        if (raw.size() != 15)
            return std::nullopt;
        return MotionEvent{
            .period      = uint16_t((int(raw[1]) << 8) | int(raw[2])) / 16.0f,
            .translation = {
                int16_t((int(raw[3]) << 8) | int(raw[4])),
                int16_t((int(raw[5]) << 8) | int(raw[6])),
                int16_t((int(raw[7]) << 8) | int(raw[8])),
            },
            .rotation = {
                int16_t((int(raw[9])  << 8) | int(raw[10])),
                int16_t((int(raw[11]) << 8) | int(raw[12])),
                int16_t((int(raw[13]) << 8) | int(raw[14])),
            }
        };
    }
}
