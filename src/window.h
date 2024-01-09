#pragma once

#include <string>
#include <memory>

#include <SDL.h>
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer.h"
#include "ImGuiFileDialog.h"

#include "chip8.h"

class Window {

public:
	const std::string WINDOW_TITLE = "CHIP-8 Emulator";
	const int WINDOW_WIDTH = 1280;
	const int WINDOW_HEIGHT = 720;
	const int GAME_WIDTH = 1024;
	const int GAME_HEIGHT = 512;

	bool Init();
	bool InitCHIP8(const char* path);
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

	bool running = false;
	CHIP8* chip8;
	bool paused;
	Uint64 delayTime, soundTime;

	SDL_Rect pixel;
	ImFont* font;
	ImFont* fontMedium;
	ImFont* fontBig;
	std::string romName = "slipperyslope.ch8";

};