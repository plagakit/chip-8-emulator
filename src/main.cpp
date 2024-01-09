#include "window.h"
#include <iostream>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

Window window;

void UpdateWindow()
{
	if (!window.IsRunning())
	{
		window.Terminate();
#ifdef __EMSCRIPTEN__
		empscripten_cancel_main_loop()
#endif
	}
	window.HandleEvents();
	window.Update();
	window.Render();
}

int main(int argc, char* args[])
{
	if (!window.Init()) 
		return 1;

	if(!window.InitCHIP8("res/slipperyslope.ch8"))
		return 1;

#ifdef __EMSCRIPTEN__
	emscripten_set_main_loop(EmscriptenLoop, 0, 1);
#else
	while (window.IsRunning())
		UpdateWindow();

	window.Terminate();
#endif

	return 0;
}

