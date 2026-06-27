  ---
  **Severity: HIGH**
  1. Correctness — NextEvent silently drops reads and busy-loops on I/O error

  while (byte != std::byte{'D'} && byte != std::byte{'K'})
      numBytes = read(this->fileDescriptor, &byte, 1);

  read() returning -1 (error) or 0 (EOF/timeout — the port is configured with VTIME=1, VMIN=0) is silently ignored. This spins at 100% CPU
  on a disconnected device. Fix: check the return value, throw or return std::optional<SpaceballEvent> on error/EOF.

  ---
  **Severity: HIGH**
  2. Architecture — Error handling uses return instead of exceptions or std::expected

  Serial::Serial has five early-return paths that leave the object in an invalid state (open fd, zero-initialized gOriginalTTYAttrs, etc.),
  with errors only printed to stderr. Callers have no way to detect failure — Spaceball::Spaceball calls write() immediately after
  construction with no validity check, so a failed open silently writes to fd -1. Fix: throw on failure, or return std::expected<Spaceball,
  std::error_code> from a factory. At minimum, add an is_open() method and check it in Spaceball::Spaceball.

  ---
  **Severity: HIGH**
  3. Correctness — D event byte-order is wrong

  In spaceball.cc:59, the translation/rotation data is decoded as:

  int16_t(int16_t(event[2 * i + 3]) << 8 | int16_t(event[2 * i + 2]))

  But the protocol doc says the data is big-endian (high byte first), so it should be event[2*i+3] as the low byte only if the device is
  little-endian — verify against the protocol PDF. The demo example does it one way, mathgame never decodes D events at all.

  ---
  **Severity: HIGH**
  4. Correctness — SpaceballEvent is too weakly typed; byte interpretation is duplicated and inconsistent

  SpaceballEvent = std::vector<std::byte> is re-parsed with identical bitmask logic in three places: spaceball.cc, demo/main.cc, and
  mathgame/main.cc. The key-bit decode in mathgame has a subtle bug: it reads keyflags from event[1]/event[2] but those indices are only
  valid for K events — it never checks event[0] == 'K' before indexing. Fix: make SpaceballEvent a proper variant type, e.g.:

  struct KeyEvent   { bool pick; std::array<bool,8> buttons; };
  struct MotionEvent { float period; std::array<int16_t,3> translation, rotation; };
  using SpaceballEvent = std::variant<KeyEvent, MotionEvent>;

  Parse once in NextEvent(), eliminate all the raw-byte bitmask code from examples.

  ---
  **Severity: MEDIUM**
  5. Idiomatic C++ — throw "string literal" in Spaceball::Spaceball

  spaceball.cc:94 throws a const char*. Fix: throw std::runtime_error.

  ---
  **Severity: MEDIUM**
  6. Idiomatic C++ — raw pthread in mathgame, use std::thread

  mathgame/main.cc uses pthread_create/pthread_join and a raw static int answer shared variable with no synchronization. With C++20 this
  should use std::jthread and std::atomic<int> (or std::binary_semaphore).

  ---
  **Severity: MEDIUM**
  7. Recent C++ — use std::bit_cast instead of manual byte-shuffling

  The int16_t reconstruction from raw bytes in spaceball.cc is error-prone. With C++20:

  std::array<std::byte, 2> raw{event[i], event[i+1]};
  int16_t val = std::bit_cast<int16_t>(raw);

  This is defined behavior, type-safe, and lets the compiler optimize to a single load on little-endian targets.

  ---
  **Severity: LOW**
  8. Idiomatic C++ — IOKitLib.h included in serial.cc but never used

  #include <IOKit/IOKitLib.h> is present but nothing from IOKit is called. Remove it.

  ---
  **Severity: LOW**
  9. Idiomatic C++ — portPath is public mutable state on Serial

  It can be changed after construction with no effect on the actual open fd. Make it const or expose it via a getter from a private member.

  ---
  **Severity: LOW**
  10. Library usability — Serial constructor takes std::string by value but Spaceball passes const char*

  Serial(std::string, unsigned int) forces a heap allocation even when called with a string literal. Fix: take std::string_view (C++17) in
  the constructor signature, or const char* if the stored member needs to outlive the call.

  ---
  **Severity: LOW**
  11. Recent C++ — std::format instead of std::setw/std::setprecision chains

  The operator<< in spaceball.cc is a wall of std::setw/std::setfill/std::hex manipulators. C++20 std::format is dramatically cleaner and
  doesn't mutate stream state:

  out << std::format("D: per={:6.4f}ms  T({:6},{:6},{:6})   R({:6},{:6},{:6})", ...);
