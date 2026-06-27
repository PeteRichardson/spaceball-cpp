# spaceball-cpp

> _A C++ library for reading events from a Spaceball 6-DOF input device._

spaceball-cpp wraps the Spaceball serial protocol in a clean C++20 API. Connect a Spaceball 2003/3003 via a USB-to-serial adapter, open the device path, and start reading key presses and 6-DOF motion events — translation and rotation — with a single blocking call per event.

Without this library, you'd need to manage serial port setup (`termios`, baud rate, flow control), send the Spaceball initialization command sequence, and parse the binary `K`/`D` packet format yourself. spaceball-cpp handles all of that.

**Limitation:** macOS only. The `Serial` class uses `IOKit/IOKitLib.h`, which is not available on Linux or Windows.

> **Status:** Maintenance mode — no new features; bug fixes only. See [spaceball-rs](https://github.com/PeteRichardson/spaceball-rs) for the actively developed Rust successor.

---

## Table of Contents

- [Features](#features)
- [Prerequisites](#prerequisites)
- [Building](#building)
- [Quick Start](#quick-start)
- [API Reference](#api-reference)
- [Event Format](#event-format)
- [Examples](#examples)
- [Known Limitations](#known-limitations)
- [License](#license)

---

## Features

- **Zero-configuration initialization** — constructor sends the full Spaceball init sequence (`CB`, `NT`, `MSSV`, `Z`, etc.) automatically
- **Blocking event loop** — `NextEvent()` blocks until a key or motion event arrives; no polling
- **Printable events** — `operator<<` renders `K` events as button bitmasks and `D` events as `T(x,y,z) R(x,y,z)` with period in milliseconds
- **Static library** — link `libspaceball.a` into your own CMake project with no runtime dependencies beyond the OS serial stack
- **Example programs** — a raw event printer (`demo`) and a math game (`mathgame`) that uses Spaceball buttons as answer keys

---

## Prerequisites

- **macOS** (required — uses `IOKit`)
- **CMake** 3.18 or later
- **C++20** compiler (Apple Clang ships with Xcode Command Line Tools)
- **Spaceball 2003 or 3003** hardware connected via a USB-to-serial adapter (e.g., FTDI)

---

## Building

```sh
git clone <repo-url>
cd spaceball-cpp
mkdir build && cd build
cmake ..
make
```

This produces:
- `build/spaceball/libspaceball.a` — the static library
- `build/spaceball-examples/demo/demo` — raw event printer
- `build/spaceball-examples/mathgame/mathgame` — interactive math game

---

## Quick Start

Find your Spaceball's serial device path:

```sh
ls /dev/tty.usbserial-*
```

Then run the demo:

```sh
./build/spaceball-examples/demo/demo
```

Expected output while moving the ball and pressing buttons:

```
D: per=50.00ms  T(   120,   -45,     0)   R(     0,     0,    33)
K: _1__________
K: ____________
D: per=50.00ms  T(     0,     0,     0)   R(     0,     0,     0)
```

---

## API Reference

### `Spaceball`

```cpp
#include "spaceball.h"

Spaceball sb("/dev/tty.usbserial-AJ03ACPV");
```

**Constructor** — opens the serial port at 9600 baud and sends the Spaceball initialization sequence. Throws `const char*` on write failure. Prints to `stderr` on port-open or `termios` errors.

```cpp
SpaceballEvent event = sb.NextEvent();
```

**`NextEvent()`** — blocks until the next `K` (key) or `D` (displacement) event arrives and returns it as a `SpaceballEvent`.

```cpp
std::cout << event << std::endl;
```

**`operator<<`** — streams a human-readable representation of any `SpaceballEvent`.

### `SpaceballEvent`

```cpp
using SpaceballEvent = std::vector<std::byte>;
```

`event[0]` is the packet type byte (`'K'` or `'D'`). Remaining bytes are the raw packet payload as described in [Event Format](#event-format).

### `Serial`

Base class for `Spaceball`. Manages the serial port file descriptor and `termios` configuration. Restored to original state on destruction.

---

## Event Format

### Key event (`K`)

Sent whenever any button is pressed or released. Encodes the full state of all nine buttons.

```
event[0]    = 'K'
event[1..2] = two-byte button state bitmask
```

Button bitmasks (applied to the 16-bit value `(event[1] << 8) | event[2]`):

| Button | Mask     |
|--------|----------|
| Pick   | `0x1000` |
| 1      | `0x0001` |
| 2      | `0x0002` |
| 3      | `0x0004` |
| 4      | `0x0008` |
| 5      | `0x0100` |
| 6      | `0x0200` |
| 7      | `0x0400` |
| 8      | `0x0800` |

`operator<<` renders this as `K: P1234____` where each character is its button label if pressed or `_` if not.

### Displacement event (`D`)

Sent when the ball is in motion, at up to 20 events per second (configured by init sequence).

```
event[0]     = 'D'
event[1..2]  = period (uint16, 1/16ths of a millisecond since last D packet)
event[3..14] = six signed 16-bit values, little-endian:
               TX, TY, TZ (translation), RX, RY, RZ (rotation)
```

`operator<<` renders this as:

```
D: per=50.00ms  T(   120,   -45,     0)   R(     0,     0,    33)
```

See `docs/sbprotocol.txt` and `docs/SpaceBall_2003-3003_Protocol.pdf` for the full protocol specification.

---

## Examples

### `demo` — print all events

```cpp
auto sb = Spaceball("/dev/tty.usbserial-AJ03ACPV");
while (true)
    std::cout << sb.NextEvent() << std::endl;
```

### `mathgame` — use buttons 1–8 as answer keys

Generates random single-digit addition problems (operands 1–4). Press the numbered button matching the answer, or Press (Pick) to quit.

```sh
./build/spaceball-examples/mathgame/mathgame
```

```
Welcome to the Math game!

Use the Spaceball buttons to answer the questions.
Press the Pick button (on the ball) to quit.

What is 3 + 2? 5 is correct!
What is 1 + 4? 3 is wrong.  Try again!
```

### Integrating into your own CMake project

```cmake
add_subdirectory(path/to/spaceball-cpp/spaceball)
target_link_libraries(your_target PRIVATE spaceball)
```

Then include:

```cpp
#include "spaceball.h"
```

---

## Known Limitations

- **macOS only** — `Serial` uses `IOKit/IOKitLib.h`; porting to Linux would require replacing that header with a POSIX-only `termios` implementation.
- **Hard-coded device path in examples** — `demo` and `mathgame` hard-code `/dev/tty.usbserial-AJ03ACPV`; you must edit `main.cc` to match your adapter's device node.
- **No escape-sequence decoding** — the Spaceball binary protocol uses two-byte escape sequences for `XON`, `XOFF`, `CR`, and `^` in data bytes. `NextEvent()` does not decode these; raw packet bytes may occasionally be misinterpreted if the 6-DOF data happens to contain those values.
- **Answers capped at 8** — `mathgame` can only accept answers 1–8 (one button per answer); questions with a sum greater than 8 have no correct answer key.

