# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build

```bash
mkdir -p build && cd build
cmake ..
make
```

The tests subdirectory (`spaceball/tests/`) is commented out in `spaceball/CMakeLists.txt`. To enable tests, uncomment `add_subdirectory(tests)` there and add GoogleTest via FetchContent (scaffolding is already commented out in the root `CMakeLists.txt`).

## Architecture

This is a C++20 CMake project providing a library and two example apps for interfacing with a [Spaceball 2003/3003](http://spacemice.org/) 6-DOF input device over a serial port.

**`spaceball/` — static library**
- `Serial` (serial.h/cc): RAII wrapper around a macOS serial port using `termios`. Opens the port, configures it for 9600 8N1 raw mode, and restores settings on destruction. macOS-specific (`IOKit`, `TIOCEXCL`).
- `Spaceball : Serial` (spaceball.h/cc): Sends the initialization string to the device on construction, then exposes `NextEvent()` which blocks until it reads a complete packet (framed by a leading `D`/`K` byte and a trailing `\r`) and returns it as `SpaceballEvent` (`std::vector<std::byte>`).
- `operator<<(SpaceballEvent)`: Decodes and pretty-prints `K` (key/button state, 9 buttons) and `D` (6-DOF translation + rotation vectors with period) packets.

**`spaceball-examples/`**
- `demo`: Reads events in a loop and prints them to stdout.
- `mathgame`: Spawns a pthread per question that blocks on `NextEvent()`. Button presses (1–8) answer addition questions; the Pick button quits.

**Protocol**: The Spaceball speaks RS-232 binary framed packets. `D` packets are 15 bytes (period + 6 × int16 6-DOF data). `K` packets carry a 16-bit button bitmask. See `docs/sbprotocol.txt` and `docs/SpaceBall_2003-3003_Protocol.pdf` for full details.

**Hard-coded device path**: Both example apps use `/dev/tty.usbserial-AJ03ACPV` — change this to match the actual port when running.
