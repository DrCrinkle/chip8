#pragma once
#include <string>
#include <cstdint>

class Chip8 {

public:
    Chip8();
    
    static const int DISPLAY_WIDTH = 64;
    static const int DISPLAY_HEIGHT = 32;

    void Cycle();
    bool loadRom(std::string filename);
    void setKey(uint8_t keynum, bool pressed) { keypad[keynum] = pressed; }
    void updateTimers() {
        if (delay_timer > 0) delay_timer--;
        if (sound_timer > 0) sound_timer--;
    }

    bool display[DISPLAY_WIDTH * DISPLAY_HEIGHT];
    bool drawflag;

private:
    uint16_t pc;            // Program counter
    uint16_t opcode;        // Current opcode
    uint16_t sp;            // Stack pointer
    uint16_t I;             // Index register
    bool keypad[16];        // Keypad state (renamed from key)

    uint8_t Mem[4096];      // Memory
    uint8_t vReg[16];       // V registers
    uint16_t stack[12];     // Stack

    uint8_t delay_timer;    // Delay timer
    uint8_t sound_timer;    // Sound timer

};

const uint8_t default_char[80] =
{
    0xF0 ,0x90, 0x90, 0x90, 0xF0, //0
    0x20, 0x60, 0x20, 0x20, 0x70, //1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, //2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, //3
    0x90, 0x90, 0xF0, 0x10, 0x10, //4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, //5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, //6
    0xF0, 0x10, 0x20, 0x40, 0x40, //7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, //8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, //9
    0xF0, 0x90, 0xF0, 0x90, 0x90, //A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, //B
    0xF0, 0x80, 0x80, 0x80, 0xF0, //C
    0xE0, 0x90, 0x90, 0x90, 0xE0, //D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, //E
    0xF0, 0x80, 0xF0, 0x80, 0x80  //F
};
