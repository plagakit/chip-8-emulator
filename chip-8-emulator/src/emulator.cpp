#include "emulator.h"

bool Emulator::Init()
{
	chip8 = new CHIP8("..\\roms\\test_keypad.ch8");
	pixel = { 0, 0, GAME_WIDTH / 64, GAME_HEIGHT / 32 };
	
	bool success = true;

	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
		success = false;
	}
	else
	{
		window = SDL_CreateWindow(WINDOW_TITLE.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
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
	// Update key states
	const Uint8* kb = SDL_GetKeyboardState(NULL);
	chip8->keyStates[0x1] = kb[SDL_SCANCODE_1];
	chip8->keyStates[0x2] = kb[SDL_SCANCODE_2];
	chip8->keyStates[0x3] = kb[SDL_SCANCODE_3];
	chip8->keyStates[0xC] = kb[SDL_SCANCODE_4];
	chip8->keyStates[0x4] = kb[SDL_SCANCODE_Q];
	chip8->keyStates[0x5] = kb[SDL_SCANCODE_W];
	chip8->keyStates[0x6] = kb[SDL_SCANCODE_E];
	chip8->keyStates[0xD] = kb[SDL_SCANCODE_R];
	chip8->keyStates[0x7] = kb[SDL_SCANCODE_A];
	chip8->keyStates[0x8] = kb[SDL_SCANCODE_S];
	chip8->keyStates[0x9] = kb[SDL_SCANCODE_D];
	chip8->keyStates[0xE] = kb[SDL_SCANCODE_F];
	chip8->keyStates[0xA] = kb[SDL_SCANCODE_Z];
	chip8->keyStates[0x0] = kb[SDL_SCANCODE_X];
	chip8->keyStates[0xB] = kb[SDL_SCANCODE_C];
	chip8->keyStates[0xF] = kb[SDL_SCANCODE_V];
	
	// Do a cycle
	chip8->Cycle();
}

void Emulator::Render()
{
	SDL_SetRenderDrawColor(renderer, 94, 75, 107, 255);
	SDL_RenderClear(renderer);
	
	// Draw game
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	pixel.x = 0;
	pixel.y = 0;
	for (int i = 0; i < 64; i++)
	{
		for (int j = 0; j < 32; j++)
		{
			pixel.x = i * (GAME_WIDTH / 64);
			pixel.y = j * (GAME_HEIGHT / 32);
			if (chip8->display[j][i])
				SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
			else
				SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
			SDL_RenderFillRect(renderer, &pixel);
		}
	}

	// Draw keys
	SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
	SDL_Rect keyRect = { 0, 512, 52, 52 };
	
	// Maps CHIP8 keyboard to their values
	int keyMap[] = {1, 2, 3, 0xC, 4, 5, 6, 0xD, 7, 8, 9, 0xE, 0xA, 0, 0xB, 0xF};
	for (int i = 0; i < 16; i++)
	{
		if (chip8->keyStates[keyMap[i]])
		{
			keyRect.x = (i % 4) * 52;
			keyRect.y = (i / 4) * 52 + 512;
			SDL_RenderFillRect(renderer, &keyRect);
		}
	}


	SDL_RenderPresent(renderer);
}


bool Emulator::IsRunning()
{
	return running;
}
