#pragma once

#include <SDL.h>
#include <string>
#include "chip8.h"

class Emulator {

public:
	const std::string WINDOW_TITLE = "CHIP-8 Emulator";
	const int SCREEN_WIDTH = 1024;
	const int SCREEN_HEIGHT = 512;

	Emulator();
	~Emulator();

	bool Init();
	void Terminate();

	void HandleEvents();
	void Update();
	void Render();

	bool IsRunning();

private:
	SDL_Window* window;
	SDL_Surface* surface;
	SDL_Renderer* renderer;
	std::unique_ptr<CHIP8> chip8;
	bool running;

};