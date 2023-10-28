#pragma once

#include <string>
#include <memory>

#include <SDL.h>
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer.h"
#include "ImGuiFileDialog.h"

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
	void UpdateCHIP8();
	void Render();

	bool IsRunning();

private:
	SDL_Window* window;
	SDL_Surface* surface;
	SDL_Renderer* renderer;

	bool running;
	CHIP8* chip8;
	bool paused;
	Uint64 delayTime, soundTime;

	SDL_Rect pixel;
	ImFont* font;
	ImFont* fontKeymap;
	ImFont* fontBig;
	std::string romName;

};