#include "emulator.h"

bool Emulator::Init()
{
	chip8 = new CHIP8("..\\roms\\slipperyslope.ch8");
	delayTime = SDL_GetTicks64();
	soundTime = SDL_GetTicks64();

	pixel = { 0, 0, GAME_WIDTH / 64, GAME_HEIGHT / 32 };
	paused = false;

	// Init SDL
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) != 0)
	{
		printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
		return false;
	}

	// Create SDL window
	window = SDL_CreateWindow(WINDOW_TITLE.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
	if (window == NULL)
	{
		printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
		return false;
	}
	
	// Create accelerated renderer
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);// | SDL_RENDERER_PRESENTVSYNC);
	if (renderer == NULL)
	{
		printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
		return false;
	}
	else
	{
		SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
		surface = SDL_GetWindowSurface(window);
	}

	// Init ImGui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsLight();

	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	font = io.Fonts->AddFontFromFileTTF("lib\\Segoe-UI-Variable.ttf", 22.0f);
	fontBig = io.Fonts->AddFontFromFileTTF("lib\\Segoe-UI-Variable.ttf", 32.0f);

	ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
	ImGui_ImplSDLRenderer_Init(renderer);

	running = true;
	return true;
}

void Emulator::Terminate()
{
	delete chip8;
	
	ImGui_ImplSDL2_Shutdown();
	ImGui_ImplSDLRenderer_Shutdown();
	ImGui::DestroyContext();

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
		ImGui_ImplSDL2_ProcessEvent(&e);
		if (e.type == SDL_QUIT)
			running = false;
	}
}

void Emulator::Update()
{	
	// UPDATE CHIP 8
	if (chip8 != nullptr && !paused)
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

		// Decrement DT/ST register if 1/60 secs (16.66 ms) has passed
		Uint64 time = SDL_GetTicks64();
		if (time - delayTime > 16 && chip8->DT > 0)
		{
			delayTime = time;
			chip8->DT--;
		}
		if (time - soundTime > 16 && chip8->ST > 0)
		{
			soundTime = time;
			chip8->ST--;
		}

		// Do a cycle
		chip8->Cycle();
	}
	
	// UPDATE GUI
	ImGui_ImplSDLRenderer_NewFrame();
	ImGui_ImplSDL2_NewFrame();
	ImGui::NewFrame();

	ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;

	// Bottom left pane
	ImGui::SetNextWindowPos(ImVec2(0, 512));
	ImGui::SetNextWindowSize(ImVec2(1280, 208));
	ImGui::Begin("LeftPane", NULL, flags);

	ImGui::End();


	// Right pane
	ImGui::SetNextWindowPos(ImVec2(1024, 0));
	ImGui::SetNextWindowSize(ImVec2(256, 720));
	ImGui::Begin("RightPane", NULL, flags);

	ImGui::ShowDemoWindow();

	// LOAD ROM BUTTON
	ImGui::PushFont(fontBig);
	bool buttonClicked = ImGui::Button("Load ROM", ImVec2(240, 50));
	ImGui::PopFont();

	if (buttonClicked)
	{
		printf("Clicked button!\n");
		ImGuiFileDialog::Instance()->OpenDialog("ChooseRom", "Choose File", ".ch8", ".");
	}
	if (ImGuiFileDialog::Instance()->Display("ChooseRom"))
	{
		if (ImGuiFileDialog::Instance()->IsOk())
		{
			std::string filePath = ImGuiFileDialog::Instance()->GetFilePathName();
			romName = ImGuiFileDialog::Instance()->GetCurrentFileName();
			chip8 = new CHIP8(filePath.c_str());
			paused = false;
		}
		ImGuiFileDialog::Instance()->Close();
	}
	ImGui::Text("%s", romName.c_str());

	// Pause/step buttons
	if (chip8 == nullptr)
	{
		ImGui::BeginDisabled();
		ImGui::Button("Pause", ImVec2(116, 40));
		ImGui::SameLine();
		ImGui::Button("Cycle", ImVec2(116, 40));
		ImGui::EndDisabled();
	}
	else
	{
		std::string pauseText = paused ? "Resume" : "Pause";
		if (ImGui::Button(pauseText.c_str(), ImVec2(116, 40)))
			paused = !paused;

		ImGui::SameLine();

		if (!paused)
			ImGui::BeginDisabled();

		if (ImGui::Button("Cycle", ImVec2(116, 40)))
			chip8->Cycle();

		if (!paused) ImGui::EndDisabled();
	}

	// PC & instructions
	if (chip8 != nullptr)
	{
		ImGui::Text("PC: 0x%03X", chip8->PC);

		ImGui::Text("Instructions");
		ImGui::BeginChild("Instructions", ImVec2(240, 472), true);
		for (std::string s : chip8->instructionList)
			ImGui::Text("%s", s.c_str());
		ImGui::EndChild();

		ImGui::Text("Stack Length: %d", chip8->stack.size());
		ImGui::Text("Stack Top:");
		if (chip8->stack.size() > 0)
		{
			ImGui::SameLine();
			ImGui::Text("0x%03X", chip8->stack.top());
		}
	}

	ImGui::End();
}

void Emulator::Render()
{
	
	SDL_SetRenderDrawColor(renderer, 94, 75, 107, 255);
	SDL_RenderClear(renderer);

	// Draw game
	if (chip8 != nullptr)
	{
		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
		pixel.x = 0;
		pixel.y = 0;
		for (int i = 0; i < 64; i++)
		{
			for (int j = 0; j < 32; j++)
			{
				if (chip8->display[j][i])
				{
					pixel.x = i * (GAME_WIDTH / 64);
					pixel.y = j * (GAME_HEIGHT / 32);
					SDL_RenderFillRect(renderer, &pixel);
				}
			}
		}

		// Draw keys
		SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
		SDL_Rect keyRect = { 0, 512, 52, 52 };

		// Maps CHIP8 keyboard to their values
		int keyMap[] = { 1, 2, 3, 0xC, 4, 5, 6, 0xD, 7, 8, 9, 0xE, 0xA, 0, 0xB, 0xF };
		for (int i = 0; i < 16; i++)
		{
			if (chip8->keyStates[keyMap[i]])
			{
				keyRect.x = (i % 4) * 52;
				keyRect.y = (i / 4) * 52 + 512;
				SDL_RenderFillRect(renderer, &keyRect);
			}
		}
	}

	ImGui::Render();
	ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());

	SDL_RenderPresent(renderer);
}


bool Emulator::IsRunning()
{
	return running;
}
