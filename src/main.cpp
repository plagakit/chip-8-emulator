#include "emulator.h"
#include <iostream>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

Emulator emulator;

void EmulatorLoop()
{
	emulator.HandleEvents();
	emulator.Update();
	emulator.Render();
}

#ifdef __EMSCRIPTEN__
void EmscriptenLoop()
{
	if (!emulator.IsRunning())
	{
		emulator.Terminate();
		emscripten_cancel_main_loop();
	}
	EmulatorLoop();
}
#endif

int main(int argc, char* args[])
{
	if(!emulator.Init("res/slipperyslope.ch8"))
		return 1;

#ifdef __EMSCRIPTEN__
	emscripten_set_main_loop(EmscriptenLoop, 0, 1);
#else
	while (emulator.IsRunning())
		EmulatorLoop();
#endif

	emulator.Terminate();

	return 0;
}

