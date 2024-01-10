#include "window.h"

#include <iostream>
#include <filesystem>

bool Window::Init() 
{
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
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

	// Init fonts
	const char *fontPath = "res/Segoe-UI-Variable.ttf";
	font = io.Fonts->AddFontFromFileTTF(fontPath, 22.0f);
	fontMedium = io.Fonts->AddFontFromFileTTF(fontPath, 28.0f);
	fontBig = io.Fonts->AddFontFromFileTTF(fontPath, 32.0f);

	ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
	ImGui_ImplSDLRenderer_Init(renderer);
}

bool Window::InitCHIP8(const char* path)
{
	chip8 = new CHIP8(path);
	delayTime = SDL_GetTicks64();
	soundTime = SDL_GetTicks64();

	romName = std::filesystem::path(path).filename().string();

	std::cout << "Initialized emulator.\n";
	paused = false;
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
	
	// UPDATE GUI - seems like this should be in Render() but ImGui needs to 
	// be "rendered" in update before actually rendering
	ImGui_ImplSDLRenderer_NewFrame();
	ImGui_ImplSDL2_NewFrame();
	ImGui::NewFrame();

	RenderKeymap();
	RenderTimers();
	RenderRegisters();
	RenderRightPane();
}

void Window::RenderKeymap()
{
	static const ImVec2 window_pos = { 0, 512 };
	static const ImVec2 window_size = { 208, 208 };
	static const char key_chars[] = { '1','2','3','4','Q','W','E','R','A','S','D','F','Z','X','C','V' };
	//static const char key_vals[] = { '1', '2', '3', 'C', '4', '5', '6', 'D', '7', '8', '9', 'E', 'A', '0', 'B', 'F' };

	ImGui::SetNextWindowPos(window_pos);
	ImGui::SetNextWindowSize(window_size);
	ImGui::Begin("KeymapPane", NULL, flags | ImGuiWindowFlags_NoBackground);
	ImGui::PushFont(fontMedium);

	if (ImGui::BeginTable("KeymapTable", 4, tableFlags, window_size))
	{
		for (int row = 0; row < 4; row++)
		{
			ImGui::TableNextRow(0, 52);
			for (int col = 0; col < 4; col++)
			{
				ImGui::TableSetColumnIndex(col);

				// Center key text in squares with shitty hack
				auto windowWidth = 52;
				auto textWidth = ImGui::CalcTextSize("O").x;
				ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f + 52 * col);

				// And then render the right key text
				ImGui::Text("%c", key_chars[row * 4 + col]);
			}
		}
		ImGui::EndTable();
	}
	ImGui::PopFont();

	ImGui::End();
}

void Window::RenderTimers()
{
	static const ImVec2 window_pos = { 208, 512 };
	static const ImVec2 window_size = { 208, 208 };
	static const ImU32 off_color = IM_COL32(65, 60, 60, 255);
	static const ImU32 on_color = IM_COL32(0, 255, 0, 255);

	ImGui::SetNextWindowPos(window_pos);
	ImGui::SetNextWindowSize(window_size);
	ImGui::Begin("TimerWindow", NULL, flags);

	if (chip8 != nullptr)
	{
		ImGui::PushFont(fontMedium);
		auto drawList = ImGui::GetWindowDrawList();

		// Delay timer
		ImGui::Text("Delay Timer: 0x%02x", chip8->DT);

		float dtProgress = (float)chip8->DT / 0xF;
		ImVec2 rectMin = ImGui::GetCursorScreenPos();
		ImVec2 rectMax = ImVec2(rectMin.x + 208, rectMin.y + 50);
		ImVec2 dtMax = ImVec2(rectMin.x + dtProgress * 208, rectMin.y + 50);

		drawList->AddRectFilled(rectMin, rectMax, off_color);
		drawList->AddRectFilled(rectMin, dtMax, on_color);

		// Sound timer
		ImGui::SetCursorScreenPos(ImVec2(rectMin.x, rectMin.y + 65));
		ImGui::Text("Sound Timer: 0x%02x", chip8->ST);

		float stProgress = (float)chip8->ST / 0xF;
		rectMin = ImGui::GetCursorScreenPos();
		rectMax = ImVec2(rectMin.x + 208, rectMin.y + 50);
		ImVec2 stMax = ImVec2(rectMin.x + stProgress * 208, rectMin.y + 50);

		drawList->AddRectFilled(rectMin, rectMax, off_color);
		drawList->AddRectFilled(rectMin, stMax, on_color);

		ImGui::PopFont();
	}
	ImGui::End();
}

void Window::RenderRegisters()
{	
	static const ImVec2 window_pos = { 208 * 2, 512 };
	static const ImVec2 window_size = { 608, 208 };

	ImGui::SetNextWindowPos(window_pos);
	ImGui::SetNextWindowSize(window_size);
	ImGui::Begin("RegisterWindow", NULL, flags);
	ImGui::PushFont(fontMedium);

	if (ImGui::BeginTable("RegistersTable", 6, tableFlags, window_size))
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
}

void Window::RenderRightPane() 
{
	static const ImVec2 window_pos = { 1024, 0 };
	static const ImVec2 window_size = { 256, 720 };
	static const int margin = 8;

	// Right pane
	ImGui::SetNextWindowPos(window_pos);
	ImGui::SetNextWindowSize(window_size);
	ImGui::Begin("RightPane", NULL, flags);

	// LOAD ROM BUTTON
	ImGui::PushFont(fontBig);
	bool buttonClicked = ImGui::Button("Load ROM", ImVec2(window_size.x - margin * 2, 50));
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
			InitCHIP8(filePath.c_str());
		}
		ImGuiFileDialog::Instance()->Close();
	}
	ImGui::Text("%s", romName.c_str());

	// Pause/step buttons
	static const ImVec2 button_size = { window_size.x / 2 - (int)(margin * 1.5), 40 };
	if (chip8 == nullptr)
	{
		ImGui::BeginDisabled();
		ImGui::Button("Pause", button_size);
		ImGui::SameLine();
		ImGui::Button("Cycle", button_size);
		ImGui::EndDisabled();
	}
	else
	{
		std::string pauseText = paused ? "Resume" : "Pause";
		if (ImGui::Button(pauseText.c_str(), button_size))
			paused = !paused;

		ImGui::SameLine();

		if (!paused)
			ImGui::BeginDisabled();

		if (ImGui::Button("Cycle", button_size))
			UpdateCHIP8();

		if (!paused) ImGui::EndDisabled();
	}

	// PC & instructions & stack
	if (chip8 != nullptr)
	{
		ImGui::Text("PC: 0x%03X", chip8->PC);

		// Instructions
		ImGui::Text("Instructions");
		ImGui::BeginChild("Instructions", ImVec2(window_size.x - margin * 2, 472), true);
		for (std::string s : chip8->instructionList)
			ImGui::Text("%s", s.c_str());
		ImGui::EndChild();

		// Stack
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

	// Render game
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

	// Render ImGui windows
	ImGui::Render();
	ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());

	SDL_RenderPresent(renderer);
}


bool Window::IsRunning()
{
	return running;
}
