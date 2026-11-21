#include <SDL2/SDL.h>
#include <array>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <ctime>
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
        pc          = 0x200; // User ROM execution starts at 0x200
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

        // 80-byte built-in hex fontset (0-F, 5 bytes each) loaded at 0x050
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
        initialize();

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

    void emulateCycle() {
        pc &= 0x0FFF;
        uint16_t opcode = (static_cast<uint16_t>(memory[pc]) << 8) | memory[(pc + 1) & 0x0FFF];

        uint8_t  x   = (opcode & 0x0F00) >> 8;
        uint8_t  y   = (opcode & 0x00F0) >> 4;
        uint8_t  n   = opcode & 0x000F;
        uint8_t  kk  = opcode & 0x00FF;
        uint16_t nnn = opcode & 0x0FFF;

        switch (opcode & 0xF000) {
            case 0x0000:
                switch (opcode & 0x00FF) {
                    case 0x00E0: // CLS
                        std::memset(gfx, 0, sizeof(gfx));
                        draw_flag = true;
                        pc += 2;
                        break;
                    case 0x00EE: // RET
                        if (sp > 0) {
                            --sp;
                            pc = stack[sp];
                        } else {
                            pc += 2;
                        }
                        break;
                    default:
                        std::cerr << "[WARN] Unknown 0x0 opcode: 0x" << std::hex << opcode << std::dec << "\n";
                        pc += 2;
                        break;
                }
                break;

            case 0x1000: // JP addr
                pc = nnn;
                break;

            case 0x2000: // CALL addr
                if (sp < 16) {
                    stack[sp] = static_cast<uint16_t>((pc + 2) & 0x0FFF);
                    ++sp;
                    pc = nnn;
                } else {
                    std::cerr << "[WARN] Stack overflow on CALL 0x" << std::hex << nnn << std::dec << "\n";
                    pc += 2;
                }
                break;

            case 0x3000: // SE Vx, byte
                pc += (V[x] == kk) ? 4 : 2;
                break;

            case 0x4000: // SNE Vx, byte
                pc += (V[x] != kk) ? 4 : 2;
                break;

            case 0x5000: // SE Vx, Vy
                if ((opcode & 0x000F) == 0x0) {
                    pc += (V[x] == V[y]) ? 4 : 2;
                } else {
                    pc += 2;
                }
                break;

            case 0x6000: // LD Vx, byte
                V[x] = kk;
                pc += 2;
                break;

            case 0x7000: // ADD Vx, byte (no carry flag)
                V[x] = static_cast<uint8_t>(V[x] + kk);
                pc += 2;
                break;

            case 0x8000:
                switch (opcode & 0x000F) {
                    case 0x0: // LD Vx, Vy
                        V[x] = V[y];
                        break;
                    case 0x1: // OR Vx, Vy
                        V[x] |= V[y];
                        break;
                    case 0x2: // AND Vx, Vy
                        V[x] &= V[y];
                        break;
                    case 0x3: // XOR Vx, Vy
                        V[x] ^= V[y];
                        break;
                    case 0x4: { // ADD Vx, Vy (VF = carry)
                        uint16_t sum = static_cast<uint16_t>(V[x]) + static_cast<uint16_t>(V[y]);
                        V[0xF] = (sum > 255) ? 1 : 0;
                        V[x]   = static_cast<uint8_t>(sum & 0xFF);
                        break;
                    }
                    case 0x5: { // SUB Vx, Vy (VF = NOT borrow)
                        V[0xF] = (V[x] >= V[y]) ? 1 : 0;
                        V[x]   = static_cast<uint8_t>(V[x] - V[y]);
                        break;
                    }
                    case 0x6: { // SHR Vx {, Vy}
                        V[0xF] = V[x] & 0x1;
                        V[x] >>= 1;
                        break;
                    }
                    case 0x7: { // SUBN Vx, Vy
                        V[0xF] = (V[y] >= V[x]) ? 1 : 0;
                        V[x]   = static_cast<uint8_t>(V[y] - V[x]);
                        break;
                    }
                    case 0xE: { // SHL Vx {, Vy}
                        V[0xF] = (V[x] & 0x80) >> 7;
                        V[x] <<= 1;
                        break;
                    }
                    default:
                        std::cerr << "[WARN] Unknown 0x8 opcode: 0x" << std::hex << opcode << std::dec << "\n";
                        break;
                }
                pc += 2;
                break;

            case 0x9000: // SNE Vx, Vy
                if ((opcode & 0x000F) == 0x0) {
                    pc += (V[x] != V[y]) ? 4 : 2;
                } else {
                    pc += 2;
                }
                break;

            case 0xA000: // LD I, addr
                I = nnn;
                pc += 2;
                break;

            case 0xB000: // JP V0, addr
                pc = static_cast<uint16_t>((nnn + V[0]) & 0x0FFF);
                break;

            case 0xC000: // RND Vx, byte
                V[x] = static_cast<uint8_t>(std::rand() & 0xFF) & kk;
                pc += 2;
                break;

            case 0xD000: { // DRW Vx, Vy, nibble
                uint8_t x_pos = V[x] % 64;
                uint8_t y_pos = V[y] % 32;
                V[0xF] = 0;

                for (uint8_t row = 0; row < n; ++row) {
                    uint8_t sprite_byte = memory[(I + row) & 0x0FFF];
                    for (uint8_t col = 0; col < 8; ++col) {
                        if ((sprite_byte & (0x80 >> col)) != 0) {
                            uint16_t px  = (x_pos + col) % 64;
                            uint16_t py  = (y_pos + row) % 32;
                            uint16_t idx = static_cast<uint16_t>(py * 64 + px);

                            if (gfx[idx] == 1) {
                                V[0xF] = 1;
                            }
                            gfx[idx] ^= 1;
                        }
                    }
                }
                draw_flag = true;
                pc += 2;
                break;
            }

            case 0xE000:
                switch (kk) {
                    case 0x9E: // SKP Vx
                        pc += (key[V[x] & 0x0F] != 0) ? 4 : 2;
                        break;
                    case 0xA1: // SKNP Vx
                        pc += (key[V[x] & 0x0F] == 0) ? 4 : 2;
                        break;
                    default:
                        std::cerr << "[WARN] Unknown 0xE opcode: 0x" << std::hex << opcode << std::dec << "\n";
                        pc += 2;
                        break;
                }
                break;

            case 0xF000:
                switch (kk) {
                    case 0x07: // LD Vx, DT
                        V[x] = delay_timer;
                        pc += 2;
                        break;
                    case 0x0A: { // LD Vx, K (wait for key press)
                        bool pressed = false;
                        for (uint8_t k = 0; k < 16; ++k) {
                            if (key[k] != 0) {
                                V[x] = k;
                                pressed = true;
                                break;
                            }
                        }
                        if (pressed) {
                            pc += 2;
                        }
                        break;
                    }
                    case 0x15: // LD DT, Vx
                        delay_timer = V[x];
                        pc += 2;
                        break;
                    case 0x18: // LD ST, Vx
                        sound_timer = V[x];
                        pc += 2;
                        break;
                    case 0x1E: // ADD I, Vx
                        I = static_cast<uint16_t>(I + V[x]);
                        pc += 2;
                        break;
                    case 0x29: // LD F, Vx
                        I = static_cast<uint16_t>(0x050 + (V[x] & 0x0F) * 5);
                        pc += 2;
                        break;
                    case 0x33: // LD B, Vx (BCD)
                        memory[I & 0x0FFF]       = V[x] / 100;
                        memory[(I + 1) & 0x0FFF] = (V[x] / 10) % 10;
                        memory[(I + 2) & 0x0FFF] = V[x] % 10;
                        pc += 2;
                        break;
                    case 0x55: // LD [I], V0..Vx
                        for (uint8_t idx = 0; idx <= x; ++idx) {
                            memory[(I + idx) & 0x0FFF] = V[idx];
                        }
                        pc += 2;
                        break;
                    case 0x65: // LD V0..Vx, [I]
                        for (uint8_t idx = 0; idx <= x; ++idx) {
                            V[idx] = memory[(I + idx) & 0x0FFF];
                        }
                        pc += 2;
                        break;
                    default:
                        std::cerr << "[WARN] Unknown 0xF opcode: 0x" << std::hex << opcode << std::dec << "\n";
                        pc += 2;
                        break;
                }
                break;

            default:
                std::cerr << "[WARN] Unknown opcode: 0x" << std::hex << opcode << std::dec << "\n";
                pc += 2;
                break;
        }
    }

    void updateTimers() {
        if (delay_timer > 0) {
            --delay_timer;
        }
        if (sound_timer > 0) {
            --sound_timer;
        }
    }
};

static void handleKeyEvent(SDL_Keycode keycode, uint8_t state, Chip8& chip8) {
    switch (keycode) {
        case SDLK_1: chip8.key[0x1] = state; break;
        case SDLK_2: chip8.key[0x2] = state; break;
        case SDLK_3: chip8.key[0x3] = state; break;
        case SDLK_4: chip8.key[0xC] = state; break;

        case SDLK_q: chip8.key[0x4] = state; break;
        case SDLK_w: chip8.key[0x5] = state; break;
        case SDLK_e: chip8.key[0x6] = state; break;
        case SDLK_r: chip8.key[0xD] = state; break;

        case SDLK_a: chip8.key[0x7] = state; break;
        case SDLK_s: chip8.key[0x8] = state; break;
        case SDLK_d: chip8.key[0x9] = state; break;
        case SDLK_f: chip8.key[0xE] = state; break;

        case SDLK_z: chip8.key[0xA] = state; break;
        case SDLK_x: chip8.key[0x0] = state; break;
        case SDLK_c: chip8.key[0xB] = state; break;
        case SDLK_v: chip8.key[0xF] = state; break;
        default: break;
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << (argc > 0 ? argv[0] : "chip8") << " <rom_path>\n";
        return 1;
    }

    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    Chip8 chip8;
    if (!chip8.loadROM(argv[1])) {
        return 1;
    }

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "[ERROR] SDL_Init failed: " << SDL_GetError() << "\n";
        return 1;
    }

    constexpr int native_w = 64;
    constexpr int native_h = 32;
    constexpr int scale    = 12; // 768x384 window

    SDL_Window* window = SDL_CreateWindow(
        "CHIP-8 Emulator",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        native_w * scale,
        native_h * scale,
        SDL_WINDOW_SHOWN
    );
    if (!window) {
        std::cerr << "[ERROR] SDL_CreateWindow failed: " << SDL_GetError() << "\n";
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(
        window, -1, SDL_RENDERER_ACCELERATED
    );
    if (!renderer) {
        std::cerr << "[ERROR] SDL_CreateRenderer failed: " << SDL_GetError() << "\n";
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    SDL_RenderSetLogicalSize(renderer, native_w, native_h);

    SDL_Texture* texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        native_w,
        native_h
    );
    if (!texture) {
        std::cerr << "[ERROR] SDL_CreateTexture failed: " << SDL_GetError() << "\n";
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    std::array<uint32_t, native_w * native_h> pixels{};

    constexpr uint32_t frame_delay_ms = 1000 / 60; // ~16 ms per 60 Hz frame
    constexpr int cycles_per_frame    = 10;       // ~600 instructions/sec
    uint32_t last_tick_ms             = SDL_GetTicks();
    bool running                      = true;

    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            } else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    running = false;
                }
                handleKeyEvent(event.key.keysym.sym, 1, chip8);
            } else if (event.type == SDL_KEYUP) {
                handleKeyEvent(event.key.keysym.sym, 0, chip8);
            }
        }

        uint32_t now_ms = SDL_GetTicks();
        if (now_ms - last_tick_ms >= frame_delay_ms) {
            last_tick_ms = now_ms;

            for (int i = 0; i < cycles_per_frame; ++i) {
                chip8.emulateCycle();
            }

            chip8.updateTimers();

            if (chip8.draw_flag) {
                for (int i = 0; i < native_w * native_h; ++i) {
                    pixels[i] = (chip8.gfx[i] != 0) ? 0xFFFFFFFFu : 0xFF000000u;
                }
                SDL_UpdateTexture(texture, nullptr, pixels.data(), native_w * sizeof(uint32_t));
                SDL_RenderClear(renderer);
                SDL_RenderCopy(renderer, texture, nullptr, nullptr);
                SDL_RenderPresent(renderer);
                chip8.draw_flag = false;
            }
        } else {
            SDL_Delay(1);
        }
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}