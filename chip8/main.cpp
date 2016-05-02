#include "chip8.h"
#include "ConsoleRenderer.h"

#include <windows.h>

int main()
{
	chip8 c8;
	if (!c8.LoadProgram("Programs\\PONG"))
	//if (!c8.LoadProgram("Programs\\BLINKY"))
	{
		return 1;
	}

	ConsoleRenderer renderer;
	
	while (true)
	{
		c8.StepEmulation();

		if (c8.NeedsRender())
		{
			auto graphics = c8.GetGraphics();
			renderer.Render(graphics);
		}


		// TODO: Read key states

		Sleep(10);
	}

	return 0;
}