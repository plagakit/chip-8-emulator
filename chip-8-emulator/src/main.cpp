#include "emulator.h"

int main(int argc, char* args[])
{
	Emulator emulator;

	if (!emulator.Init())
		return 1;
	else
	{
		while (emulator.IsRunning())
		{
			emulator.HandleEvents();
			emulator.Update();
			emulator.Render();
		}
	}

	emulator.Terminate();

	return 0;
}