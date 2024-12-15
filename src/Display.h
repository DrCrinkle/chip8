#pragma once
#include "Chip8.h"
#include "SDL.h"

class Display
{
public:
    Display();
    ~Display();

    void updateFrame(bool display[64][32]);

private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture;
};