#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdint>
#include <iomanip>
#include "Chip8.h"

Chip8::Chip8()
{
    pc = 0x200;
    opcode = 0;
    sp = 0;
    I = 0;
    drawflag = false;

    std::memset(display, false, DISPLAY_WIDTH * DISPLAY_HEIGHT * sizeof(bool)); // clear display
    std::memset(stack, 0, sizeof(stack)); // clear stack
    std::memset(vReg, 0, sizeof(vReg)); // clear registers
    std::memset(Mem, 0, sizeof(Mem)); // clear memory
    std::memset(keypad, false, sizeof(keypad)); // Initialize all keys as not pressed

    // Load fontset into memory
    for(int i = 0; i < 80; i++) {
        Mem[i] = default_char[i];
    }

    delay_timer = 0;
    sound_timer = 0;
}

void Chip8::Cycle() {
    if (pc >= 4096) {
        std::cerr << "Error: Program counter out of bounds: 0x" << std::hex << pc << std::endl;
        exit(1);
    }

    // Read opcode as unsigned chars to ensure proper byte handling
    unsigned char msb = static_cast<unsigned char>(Mem[pc]);
    unsigned char lsb = static_cast<unsigned char>(Mem[pc + 1]);
    opcode = (msb << 8) | lsb;
    
    std::cout << "PC: 0x" << std::hex << pc 
              << " Opcode bytes: " << std::hex << std::setw(2) << std::setfill('0') 
              << static_cast<int>(msb) << " " 
              << static_cast<int>(lsb)
              << " Opcode: 0x" << std::hex << std::setw(4) << std::setfill('0') << opcode 
              << " I: 0x" << std::hex << I 
              << " SP: " << std::dec << sp << std::endl;

    uint16_t old_pc = pc;
    pc += 2;

    // here we take apart our opcode
    const uint16_t nnn = opcode & 0x0FFF;
    const uint8_t x = (opcode & 0x0F00) >> 8;
    const uint8_t y = (opcode & 0x00F0) >> 4;
    const uint8_t n = opcode & 0x000F;
    const uint8_t kk = opcode & 0x00FF;

    switch (opcode & 0xF000) {
        case 0x0000: {
            switch (opcode & 0x00FF) {
                case 0x00E0: // clear display
                    std::memset(display, false, sizeof(display));
                    drawflag = true;
                    break;
                case 0x00EE: // return from subroutine
                    if (sp > 0) {
                        pc = stack[--sp];
                    } else {
                        std::cerr << "Stack underflow at PC: 0x" << std::hex << old_pc << std::endl;
                        exit(1);
                    }
                    break;
                default:
                    std::cerr << "Unknown opcode 0x" << std::hex << opcode << " at PC: 0x" << old_pc << std::endl;
                    exit(1);
                    break;
            }
            break;
        }
        case 0x1000: // Jump to address NNN
            pc = nnn;
            break;
        case 0x2000: // Call subroutine at NNN
            if (sp >= 12) {
                std::cerr << "Stack overflow at PC: 0x" << std::hex << old_pc << std::endl;
                exit(1);
            }
            stack[sp++] = pc;
            pc = nnn;
            break;
        case 0x3000: // Skip next if VX == NN
            if (vReg[x] == kk)
                pc += 2;
            break;
        case 0x4000: // Skip next if VX != NN
            if (vReg[x] != kk)
                pc += 2;
            break;
        case 0x5000: // Skip next if VX == VY
            if (vReg[x] == vReg[y])
                pc += 2;
            break;
        case 0x6000: // Set VX to NN
            vReg[x] = kk;
            break;
        case 0x7000: // Add NN to VX
            vReg[x] += kk;
            break;
        case 0x8000: {
            switch (n) {
                case 0x0000: // Set VX to VY
                    vReg[x] = vReg[y];
                    break;
                case 0x0001: // Set VX to VX OR VY
                    vReg[x] |= vReg[y];
                    break;
                case 0x0002: // Set VX to VX AND VY
                    vReg[x] &= vReg[y];
                    break;
                case 0x0003: // Set VX to VX XOR VY
                    vReg[x] ^= vReg[y];
                    break;
                case 0x0004: { // Add VY to VX with carry
                    uint16_t sum = vReg[x] + vReg[y];
                    vReg[0xF] = (sum > 0xFF) ? 1 : 0;
                    vReg[x] = sum & 0xFF;
                    break;
                }
                case 0x0005: { // Subtract VY from VX with borrow
                    vReg[0xF] = (vReg[x] >= vReg[y]) ? 1 : 0;
                    vReg[x] -= vReg[y];
                    break;
                }
                case 0x0006: { // Shift VX right, VF = LSB
                    vReg[0xF] = vReg[x] & 0x1;
                    vReg[x] >>= 1;
                    break;
                }
                case 0x0007: { // Set VX to VY - VX with borrow
                    vReg[0xF] = (vReg[y] >= vReg[x]) ? 1 : 0;
                    vReg[x] = vReg[y] - vReg[x];
                    break;
                }
                case 0x000E: { // Shift VX left, VF = MSB
                    vReg[0xF] = (vReg[x] & 0x80) >> 7;
                    vReg[x] <<= 1;
                    break;
                }
                default:
                    std::cerr << "Unknown opcode 0x" << std::hex << opcode << " at PC: 0x" << old_pc << std::endl;
                    exit(1);
            }
            break;
        }
        case 0x9000: // Skip next if VX != VY
            if (vReg[x] != vReg[y])
                pc += 2;
            break;
        case 0xA000: // Set I to NNN
            I = nnn;
            break;
        case 0xB000: // Jump to NNN + V0
            pc = nnn + vReg[0];
            break;
        case 0xC000: // Set VX to random AND NN
            vReg[x] = (rand() % 256) & kk;
            break;
        case 0xD000: { // Draw sprite
            vReg[0xF] = 0;
            for (int yline = 0; yline < n; yline++) {
                if (I + yline >= 4096) {
                    std::cerr << "Memory access out of bounds at PC: 0x" << std::hex << old_pc << std::endl;
                    exit(1);
                }
                uint8_t pixel = Mem[I + yline];
                for (int xline = 0; xline < 8; xline++) {
                    if ((pixel & (0x80 >> xline)) != 0) {
                        int x_coord = (vReg[x] + xline) % DISPLAY_WIDTH;
                        int y_coord = (vReg[y] + yline) % DISPLAY_HEIGHT;
                        int index = y_coord * DISPLAY_WIDTH + x_coord;
                        if (display[index])
                            vReg[0xF] = 1;
                        display[index] ^= true;
                    }
                }
            }
            drawflag = true;
            break;
        }
        case 0xE000: {
            switch (kk) {
                case 0x009E: // Skip if key VX pressed
                    if (keypad[vReg[x]])
                        pc += 2;
                    break;
                case 0x00A1: // Skip if key VX not pressed
                    if (!keypad[vReg[x]])
                        pc += 2;
                    break;
                default:
                    std::cerr << "Unknown opcode 0x" << std::hex << opcode << " at PC: 0x" << old_pc << std::endl;
                    exit(1);
            }
            break;
        }
        case 0xF000: {
            switch (kk) {
                case 0x0007: // Set VX to delay timer
                    vReg[x] = delay_timer;
                    break;
                case 0x000A: { // Wait for key press
                    bool keyPress = false;
                    for (int i = 0; i < 16; i++) {
                        if (keypad[i]) {
                            vReg[x] = i;
                            keyPress = true;
                            break;
                        }
                    }
                    if (!keyPress)
                        pc -= 2;
                    break;
                }
                case 0x0015: // Set delay timer to VX
                    delay_timer = vReg[x];
                    break;
                case 0x0018: // Set sound timer to VX
                    sound_timer = vReg[x];
                    break;
                case 0x001E: // Add VX to I
                    I += vReg[x];
                    break;
                case 0x0029: // Set I to sprite location for digit VX
                    I = vReg[x] * 5;
                    break;
                case 0x0033: // Store BCD of VX
                    if (I + 2 >= 4096) {
                        std::cerr << "Memory access out of bounds at PC: 0x" << std::hex << old_pc << std::endl;
                        exit(1);
                    }
                    Mem[I] = vReg[x] / 100;
                    Mem[I + 1] = (vReg[x] / 10) % 10;
                    Mem[I + 2] = vReg[x] % 10;
                    break;
                case 0x0055: // Store V0 to VX in memory
                    if (I + x >= 4096) {
                        std::cerr << "Memory access out of bounds at PC: 0x" << std::hex << old_pc << std::endl;
                        exit(1);
                    }
                    for (int i = 0; i <= x; i++)
                        Mem[I + i] = vReg[i];
                    break;
                case 0x0065: // Load V0 to VX from memory
                    if (I + x >= 4096) {
                        std::cerr << "Memory access out of bounds at PC: 0x" << std::hex << old_pc << std::endl;
                        exit(1);
                    }
                    for (int i = 0; i <= x; i++)
                        vReg[i] = Mem[I + i];
                    break;
                default:
                    std::cerr << "Unknown opcode 0x" << std::hex << opcode << " at PC: 0x" << old_pc << std::endl;
                    exit(1);
            }
            break;
        }
        default:
            std::cerr << "Unknown opcode 0x" << std::hex << opcode << " at PC: 0x" << old_pc << std::endl;
            exit(1);
    }
}

bool Chip8::loadRom(std::string filename)
{
    std::ifstream rom;
    rom.open(filename, std::ios::binary);

    if (!rom.is_open()) {
        std::cerr << "Unable to open ROM: " << filename << std::endl;
        return false;
    }

    // get length of rom
    rom.seekg(0, rom.end);
    int length = (int)rom.tellg();
    rom.seekg(0, rom.beg);

    if (length > 4096 - 0x200) {
        std::cerr << "ROM size too large for buffer: " << length << " bytes" << std::endl;
        return false;
    }

    // Clear memory before loading ROM
    std::memset(&Mem[0x200], 0, 4096 - 0x200);

    // load rom into memory at 0x200
    unsigned char* buffer = new unsigned char[length];
    rom.read(reinterpret_cast<char*>(buffer), length);
    
    if (!rom) {
        std::cerr << "Error reading ROM: only " << rom.gcount() << " bytes could be read" << std::endl;
        delete[] buffer;
        return false;
    }

    // Copy ROM data and print first few bytes for debugging
    std::memcpy(&Mem[0x200], buffer, length);
    std::cout << "First few bytes of ROM: ";
    for (int i = 0; i < std::min(8, length); i++)
        std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(static_cast<unsigned char>(Mem[0x200 + i])) << " ";
    std::cout << std::endl;

    delete[] buffer;
    std::cout << "Successfully loaded ROM: " << filename << " (" << length << " bytes)" << std::endl;
    return true;
}
