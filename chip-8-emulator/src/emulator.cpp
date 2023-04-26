#include "emulator.h"

Emulator::Emulator()
{
	window = NULL;
	surface = NULL;
	renderer = NULL;
	chip8 = nullptr;
	running = false;
}

Emulator::~Emulator()
{
	Terminate();
}

bool Emulator::Init()
{
	chip8 = std::make_unique<CHIP8>(CHIP8("..\\roms\\test_opcode.ch8"));
	
	bool success = true;

	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
		success = false;
	}
	else
	{
		window = SDL_CreateWindow(WINDOW_TITLE.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
		if (window == NULL)
		{
			printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
			success = false;
		}
		else
		{
			// Create vsync accelerated renderer
			renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
			if (renderer == NULL)
			{
				printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
				success = false;
			}
			else
			{
				SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);

				// Initialize SDL_image
				/*int imgFlags = IMG_INIT_PNG;
				//if (!(IMG_Init(imgFlags) & imgFlags))
				{
					printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
					success = false;
				}
				else*/
				surface = SDL_GetWindowSurface(window);
			}
		}
	}

	running = success;
	return success;
}

void Emulator::Terminate()
{
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	renderer = NULL;
	window = NULL;

	SDL_Quit();
}

void Emulator::HandleEvents()
{
	SDL_Event e;
	while (SDL_PollEvent(&e))
	{
		if (e.type == SDL_QUIT)
			running = false;
	}
}

void Emulator::Update()
{
	chip8->Update();
}

void Emulator::Render()
{

}

bool Emulator::IsRunning()
{
	return running;
}
