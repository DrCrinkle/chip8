#include "Display.h"
#include "Chip8.h"
#include <iostream>

#define SCALE 10  // Scale up the display by 10x

Display::Display()
{
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
		exit(EXIT_FAILURE);
	}

	window = SDL_CreateWindow("Chip8 Emulator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, DISPLAY_WIDTH * SCALE, DISPLAY_HEIGHT * SCALE, SDL_WINDOW_SHOWN);
	if (window == nullptr) {
		std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
		SDL_Quit();
		exit(EXIT_FAILURE);
	}

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (renderer == nullptr) {
		SDL_DestroyWindow(window);
		std::cerr << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
		SDL_Quit();
		exit(EXIT_FAILURE);
	}
	
	texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, DISPLAY_WIDTH, DISPLAY_HEIGHT);
	if (texture == nullptr) {
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);
		std::cerr << "SDL_CreateTexture Error: " << SDL_GetError() << std::endl;
		SDL_Quit();
		exit(EXIT_FAILURE);
	}

	// Set render scale to match window size
	SDL_RenderSetScale(renderer, SCALE, SCALE);
}

Display::~Display()
{
	if (texture) SDL_DestroyTexture(texture);
	if (renderer) SDL_DestroyRenderer(renderer);
	if (window) SDL_DestroyWindow(window);
	SDL_Quit();
}

void Display::updateFrame(const bool* display) {
	// Create a 1D array for pixel data
	uint32_t pixels[DISPLAY_WIDTH * DISPLAY_HEIGHT];

	// Convert the 1D display array to pixel array
	for (int i = 0; i < DISPLAY_WIDTH * DISPLAY_HEIGHT; ++i) {
		pixels[i] = display[i] ? 0xFFFFFFFF : 0x000000FF;
	}

	// Update the texture with the pixel data
	SDL_UpdateTexture(texture, NULL, pixels, DISPLAY_WIDTH * sizeof(uint32_t));

	// Clear the renderer
	SDL_RenderClear(renderer);

	// Copy the texture to the renderer
	SDL_RenderCopy(renderer, texture, NULL, NULL);

	// Present the renderer
	SDL_RenderPresent(renderer);
}
