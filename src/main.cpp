#include <iostream>
#include <map>
#include "Display.h"

// CHIP-8 keypad layout:
// 1 2 3 C        Maps to:    1 2 3 4
// 4 5 6 D                    Q W E R
// 7 8 9 E                    A S D F
// A 0 B F                    Z X C V

const std::map<SDL_Keycode, int> KEYMAP = {
    {SDLK_1, 0x1}, {SDLK_2, 0x2}, {SDLK_3, 0x3}, {SDLK_4, 0xC},
    {SDLK_q, 0x4}, {SDLK_w, 0x5}, {SDLK_e, 0x6}, {SDLK_r, 0xD},
    {SDLK_a, 0x7}, {SDLK_s, 0x8}, {SDLK_d, 0x9}, {SDLK_f, 0xE},
    {SDLK_z, 0xA}, {SDLK_x, 0x0}, {SDLK_c, 0xB}, {SDLK_v, 0xF}
};

int main(int argc, char *argv[]) {
    std::string filename = "IBM Logo.ch8";
    if(argc > 1) {
        filename = argv[1];
    }

    Chip8 emulator;
    Display display;

    if (!emulator.loadRom(filename)) {
        std::cerr << "Failed to load ROM: " << filename << std::endl;
        return 1;
    }

    SDL_Event event;
    bool quit = false;

    while (!quit) {
        // Handle SDL events
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    quit = true;
                    break;
                case SDL_KEYDOWN: {
                    auto it = KEYMAP.find(event.key.keysym.sym);
                    if (it != KEYMAP.end()) {
                        emulator.setKey(it->second, true);
                    }
                    break;
                }
                case SDL_KEYUP: {
                    auto it = KEYMAP.find(event.key.keysym.sym);
                    if (it != KEYMAP.end()) {
                        emulator.setKey(it->second, false);
                    }
                    break;
                }
            }
        }

        // Run one CPU cycle
        try {
            emulator.Cycle();
            
            // Update timers at 60Hz
            static Uint32 lastTimerUpdate = 0;
            Uint32 currentTime = SDL_GetTicks();
            if (currentTime - lastTimerUpdate >= 1000/60) {
                emulator.updateTimers();
                lastTimerUpdate = currentTime;
            }
        } catch (const std::exception& e) {
            std::cerr << "Emulation error: " << e.what() << std::endl;
            return 1;
        }

        // Update display if needed
        if (emulator.drawflag) {
            display.updateFrame(emulator.display);
            emulator.drawflag = false;
        }

        // Add a small delay to not run too fast (approximately 500Hz)
        SDL_Delay(2);
    }

    return 0;
}