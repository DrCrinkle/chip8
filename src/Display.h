#pragma once
#include "Chip8.h"
#include "SDL.h"

class Display
{
public:
    Display();
    ~Display();

    static const int DISPLAY_WIDTH = 64;
    static const int DISPLAY_HEIGHT = 32;

    void updateFrame(const bool* display);

private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture;
};