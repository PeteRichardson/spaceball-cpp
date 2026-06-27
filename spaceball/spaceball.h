#ifndef SPACEBALL_H
#define SPACEBALL_H

#include "serial.h"
#include <array>
#include <cstdint>
#include <optional>
#include <variant>
#include <iostream>

struct KeyEvent {
    bool pick;
    std::array<bool, 8> buttons;  // buttons[0] = button 1, ..., buttons[7] = button 8
};

struct MotionEvent {
    float period;                  // ms since last D event (device sends 1/16ths of a ms)
    std::array<int16_t, 3> translation;
    std::array<int16_t, 3> rotation;
};

using SpaceballEvent = std::variant<KeyEvent, MotionEvent>;

std::ostream& operator<<(std::ostream&, const KeyEvent&);
std::ostream& operator<<(std::ostream&, const MotionEvent&);
std::ostream& operator<<(std::ostream&, const SpaceballEvent&);

class Spaceball : public Serial {
public:
    Spaceball(const char*);
    std::optional<SpaceballEvent> NextEvent(void);
};

#endif
