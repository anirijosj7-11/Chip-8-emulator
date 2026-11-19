# CHIP-8 Emulator in C++

A lightweight, cross-platform [CHIP-8](https://en.wikipedia.org/wiki/CHIP-8) interpreter written in modern C++ (C++17/C++20) using SDL2 for graphics, input, and audio. Built from scratch to explore low-level computer architecture, fetch-decode-execute cycles, and hardware emulation.

![Gameplay Demo](assets/demo.gif)

## Features

* **Complete Opcode Support:** Implements all 35 standard CHIP-8 instructions.
* **60Hz Timers & Audio:** Accurate delay and sound timer decrements at 60Hz with synthesized square-wave buzzer audio.
* **Configurable Execution Speed:** Adjustable CPU clock cycles per frame to support fast-paced and classic ROMs.
* **Crisp Scaling:** Renders the native 64x32 monochrome display at customizable window resolutions.
* **Quirk Toggles (Optional):** Supports toggleable shift (`8XY6`/`8XYE`) and memory (`FX55`/`FX65`) behaviors for compatibility across COSMAC VIP and SUPER-CHIP ROMs.

## Keypad Mapping

The original CHIP-8 used a 16-key hexadecimal keypad (`0x0`–`0xF`), mapped to a standard QWERTY keyboard as follows:

| CHIP-8 Keypad | Keyboard Mapping |
| :--- | :--- |
| `1` `2` `3` `C` | `1` `2` `3` `4` |
| `4` `5` `6` `D` | `Q` `W` `E` `R` |
| `7` `8` `9` `E` | `A` `S` `D` `F` |
| `A` `0` `B` `F` | `Z` `X` `C` `V` |

## Architecture Overview

* **Memory:** 4KB (`4096` bytes) of RAM. Addresses `0x000`–`0x1FF` are reserved for the interpreter and built-in fontset (`0x050`–`0x09F`), while ROMs load starting at `0x200`.
* **Registers:** 16 8-bit general-purpose registers (`V0`–`VF`, with `VF` acting as the carry/collision flag), a 16-bit Index register (`I`), and a 16-bit Program Counter (`PC`).
* **Stack:** 16-level call stack with an 8-bit Stack Pointer (`SP`) for subroutine returns.
* **Decode Strategy:** Uses a function pointer lookup table (or `switch`-based nibble decoding) for fast O(1) instruction dispatch.

## Getting Started

### Prerequisites

* A C++17 (or newer) compatible compiler (`GCC`, `Clang`, or `MSVC`)
* [CMake](https://cmake.org) (3.15+)
* [SDL2](https://libsdl.org) development libraries

### Building from Source

```bash
# Clone the repository
git clone https://github.com
cd chip8-emulator

# Configure and build with CMake
mkdir build && cd build
cmake ..
cmake --build .
```

### Usage

Pass the path to a valid `.ch8` ROM file via the command line:

```bash
./chip8 path/to/rom.ch8
```

**Optional Controls:**
* `P` — Pause / Resume emulation
* `R` — Reset current ROM
* `Esc` — Quit

## Project Structure

```text
├── src/
│   ├── main.cpp        # Entry point, SDL2 window & main loop
│   ├── chip8.cpp/.hpp  # CPU state, memory, and opcode execution
│   └── platform.cpp    # Graphics rendering, audio, and input polling
├── roms/               # Public domain test ROMs (e.g., Timendus test suite)
├── CMakeLists.txt
└── README.md
```

## Testing & Verification

Verified against standard CHIP-8 test suites:
* **IBM Logo** — Basic opcode and sprite rendering verification
* **BC_test / Corax89** — Arithmetic, logic, and flag instruction checks
* **Timendus CHIP-8 Test Suite** — Keypad, quirks, and flag edge-case validation

## References & Resources

* [Cowgod's Chip-8 Technical Reference](http://free.fr)
* [Tobias V. Langhoff – Guide to Making a CHIP-8 Emulator](https://github.io)
* [Austin Morlan – Building a CHIP-8 Emulator in C++](https://austinmorlan.com/posts/chip8_emulator/)

## License

Distributed under the MIT License. See `LICENSE` for more information.
