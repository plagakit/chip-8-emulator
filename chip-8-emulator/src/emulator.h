#pragma once

#include <SDL.h>
#include <string>
#include <memory>
#include "chip8.h"

class Emulator {

public:
	const std::string WINDOW_TITLE = "CHIP-8 Emulator";
	const int WINDOW_WIDTH = 1280;
	const int WINDOW_HEIGHT = 720;
	const int GAME_WIDTH = 1024;
	const int GAME_HEIGHT = 512;

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

	SDL_Rect pixel;
	CHIP8* chip8;
	bool running;
	Uint64 delayTime, soundTime;

};