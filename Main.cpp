#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>

class Chip8 {
public:
    uint8_t  memory[4096];
    uint8_t  V[16];
    uint16_t I;
    uint16_t pc;

    uint16_t stack[16];
    uint8_t  sp;

    uint8_t  gfx[64 * 32];
    uint8_t  delay_timer;
    uint8_t  sound_timer;
    uint8_t  key[16];
    bool     draw_flag;

    Chip8() {
        initialize();
    }

    void initialize() {
        pc          = 0x200; // ROM execution starts at 0x200
        I           = 0;
        sp          = 0;
        delay_timer = 0;
        sound_timer = 0;
        draw_flag   = true;

        std::memset(memory, 0, sizeof(memory));
        std::memset(V, 0, sizeof(V));
        std::memset(stack, 0, sizeof(stack));
        std::memset(gfx, 0, sizeof(gfx));
        std::memset(key, 0, sizeof(key));

        // Load standard 80-byte fontset (16 chars * 5 bytes) at 0x050
        constexpr uint8_t fontset[80] = {
            0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
            0x20, 0x60, 0x20, 0x20, 0x70, // 1
            0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
            0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
            0x90, 0x90, 0xF0, 0x10, 0x10, // 4
            0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
            0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
            0xF0, 0x10, 0x20, 0x40, 0x40, // 7
            0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
            0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
            0xF0, 0x90, 0xF0, 0x90, 0x90, // A
            0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
            0xF0, 0x80, 0x80, 0x80, 0xF0, // C
            0xE0, 0x90, 0x90, 0x90, 0xE0, // D
            0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
            0xF0, 0x80, 0xF0, 0x80, 0x80  // F
        };

        for (int i = 0; i < 80; ++i) {
            memory[0x050 + i] = fontset[i];
        }
    }
    bool loadROM(const char* filename) {
        initialize(); // Start with a clean state

        std::ifstream file(filename, std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            std::cerr << "[ERROR] Failed to open ROM: " << filename << "\n";
            return false;
        }

        std::streamsize size = file.tellg();
        constexpr std::streamsize max_rom_size = 4096 - 0x200; // 3584 bytes

        if (size <= 0) {
            std::cerr << "[ERROR] ROM file is empty.\n";
            return false;
        }
        if (size > max_rom_size) {
            std::cerr << "[ERROR] ROM too large (" << size
                      << " bytes). Max allowed: " << max_rom_size << " bytes.\n";
            return false;
        }

        file.seekg(0, std::ios::beg);
        if (!file.read(reinterpret_cast<char*>(&memory[0x200]), size)) {
            std::cerr << "[ERROR] Failed while reading ROM bytes.\n";
            return false;
        }

        std::cout << "[INFO] Loaded " << size << " bytes into memory at 0x200.\n";
        return true;
    }
};