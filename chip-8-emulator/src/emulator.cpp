#include "emulator.h"

bool Emulator::Init()
{
	chip8 = new CHIP8("..\\roms\\test_opcode.ch8");
	pixel = { 0, 0, SCREEN_WIDTH / 64, SCREEN_HEIGHT / 32 };
	
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
	delete chip8;
	
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	renderer = NULL;
	window = NULL;

	SDL_Quit();
}

void Emulator::HandleEvents()
{
	SDL_Event e;
	while (SDL_PollEvent(&e) != 0)
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
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);
	
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	pixel.x = 0;
	pixel.y = 0;
	for (int i = 0; i < 64; i++)
	{
		for (int j = 0; j < 32; j++)
		{
			if (chip8->display[j][i])
			{
				pixel.x = i * (SCREEN_WIDTH / 64);
				pixel.y = j * (SCREEN_HEIGHT / 32);
				SDL_RenderFillRect(renderer, &pixel);
			}
		}
	}

	SDL_RenderPresent(renderer);
}

bool Emulator::IsRunning()
{
	return running;
}
