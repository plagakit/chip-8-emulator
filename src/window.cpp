#include "window.h"

#include <iostream>

bool Window::Init(const char* path)
{
	chip8 = new CHIP8(path);
	delayTime = SDL_GetTicks64();
	soundTime = SDL_GetTicks64();

	pixel = { 0, 0, GAME_WIDTH / 64, GAME_HEIGHT / 32 };
	paused = false;

	// Init SDL
	std::cout << "About to initialize SDL...\n";
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) != 0)
	{
		std::cout << "SDL could not initialize! SDL Error: " << SDL_GetError() << "\n";
		return false;
	}

	// Create SDL window
	window = SDL_CreateWindow(WINDOW_TITLE.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
	if (window == NULL)
	{
		std::cout << "Window could not be created! SDL Error: " << SDL_GetError() << "\n";
		return false;
	}
	
	// Create accelerated renderer
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);// | SDL_RENDERER_PRESENTVSYNC);
	if (renderer == NULL)
	{
		std::cout << "Renderer could not be created! SDL Error: " << SDL_GetError() << "\n";
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
	char fontPath[] = "res/Segoe-UI-Variable.ttf";
	font = io.Fonts->AddFontFromFileTTF(fontPath, 22.0f);
	fontMedium = io.Fonts->AddFontFromFileTTF(fontPath, 28.0f);
	fontBig = io.Fonts->AddFontFromFileTTF(fontPath, 32.0f);

	ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
	ImGui_ImplSDLRenderer_Init(renderer);

	std::cout << "Initialized emulator.\n";
	running = true;
	return true;
}

void Window::Terminate()
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

void Window::HandleEvents()
{
	SDL_Event e;
	while (SDL_PollEvent(&e) != 0)
	{
		ImGui_ImplSDL2_ProcessEvent(&e);
		if (e.type == SDL_QUIT)
			running = false;
	}
}

void Window::UpdateCHIP8()
{
	// UPDATE CHIP 8
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

void Window::Update()
{	
	if (chip8 != nullptr && !paused)
		UpdateCHIP8();
	
	// UPDATE GUI
	ImGui_ImplSDLRenderer_NewFrame();
	ImGui_ImplSDL2_NewFrame();
	ImGui::NewFrame();

	ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;

	// Bottom left keymap pane
	ImGui::SetNextWindowPos(ImVec2(0, 512));
	ImGui::SetNextWindowSize(ImVec2(208, 208));
	ImGui::Begin("KeymapPane", NULL, flags 
		| ImGuiWindowFlags_NoBackground 
		| ImGuiWindowFlags_NoScrollbar 
		| ImGuiWindowFlags_NoScrollWithMouse);
	ImGui::PushFont(fontMedium);

	char keys[] = { '1','2','3','4','Q','W','E','R','A','S','D','F','Z','X','C','V'};
	char chars[] = { '1', '2', '3', 'C', '4', '5', '6', 'D', '7', '8', '9', 'E', 'A', '0', 'B', 'F' };
	ImGuiTableFlags tableFlags = ImGuiTableFlags_SizingStretchSame;

	if (ImGui::BeginTable("KeymapTable", 4, tableFlags, ImVec2(208, 208)))
	{
		for (int row = 0; row < 4; row++)
		{
			
			ImGui::TableNextRow(0, 52);
			for (int col = 0; col < 4; col++)
			{
				ImGui::TableSetColumnIndex(col);

				// Center text
				auto windowWidth = 52;
				auto textWidth = ImGui::CalcTextSize("O").x;
				ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f + 52 * col);

				ImGui::Text("%c", keys[row * 4 + col]);
			}
		}
		ImGui::EndTable();
	}
	ImGui::PopFont();

	ImGui::End();


	// Timers
	ImGui::SetNextWindowPos(ImVec2(208, 512));
	ImGui::SetNextWindowSize(ImVec2(208, 208));
	ImGui::Begin("TimerWindow", NULL, flags);

	if (chip8 != nullptr)
	{
		ImGui::PushFont(fontMedium);
		auto drawList = ImGui::GetWindowDrawList();

		ImGui::Text("Delay Timer: 0x%02x", chip8->DT);

		float dtProgress = (float)chip8->DT / 15;
		ImVec2 p0 = ImGui::GetCursorScreenPos();
		ImVec2 p1 = ImVec2(p0.x + dtProgress * 208, p0.y + 50);
		ImVec2 p1n = ImVec2(p0.x + 208, p0.y + 50);
		drawList->AddRectFilled(p0, p1n, IM_COL32(65, 60, 60, 255));
		drawList->AddRectFilled(p0, p1, IM_COL32(0, 255, 0, 255));

		ImGui::SetCursorScreenPos(ImVec2(p0.x, p0.y + 65));
		ImGui::Text("Sound Timer: 0x%02x", chip8->ST);

		float stProgress = (float)chip8->ST / 15;
		ImVec2 p2 = ImGui::GetCursorScreenPos();
		ImVec2 p3 = ImVec2(p2.x + stProgress * 208, p2.y + 50);
		ImVec2 p3n = ImVec2(p2.x + 208, p2.y + 50);
		drawList->AddRectFilled(p2, p3n, IM_COL32(65, 60, 60, 255));
		drawList->AddRectFilled(p2, p3, IM_COL32(0, 255, 0, 255));

		ImGui::PopFont();
	}
	ImGui::End();

	// Registers
	ImGui::SetNextWindowPos(ImVec2(208 * 2, 512));
	ImGui::SetNextWindowSize(ImVec2(608, 208));
	ImGui::Begin("RegisterWindow", NULL, flags | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
	ImGui::PushFont(fontMedium);

	if (ImGui::BeginTable("RegistersTable", 6, tableFlags, ImVec2(608, 208)))
	{
		for (int row = 0; row < 3; row++)
		{

			ImGui::TableNextRow(0, 64);
			for (int col = 0; col < 6; col++)
			{
				ImGui::TableSetColumnIndex(col);

				if (row * 6 + col > 0xF)
					continue;

				int i = row * 6 + col;
				unsigned char val = 'a';
				if (chip8 != nullptr)
					val = chip8->V[i];

				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10);
				ImGui::Text("V[%01X]\n0x%02X", i, val);
			}
		}
		ImGui::EndTable();
	}
	ImGui::PopFont();
	ImGui::End();

	// Sprite



	// Right pane
	ImGui::SetNextWindowPos(ImVec2(1024, 0));
	ImGui::SetNextWindowSize(ImVec2(256, 720));
	ImGui::Begin("RightPane", NULL, flags);

	//ImGui::ShowDemoWindow();

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
			UpdateCHIP8();

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

void Window::Render()
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


bool Window::IsRunning()
{
	return running;
}
