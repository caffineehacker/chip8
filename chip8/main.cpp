#include "chip8.h"

int main()
{
	chip8 c8;
	if (!c8.LoadProgram("Programs\\PONG"))
	{
		return 1;
	}
	
	while (true)
	{
		c8.StepEmulation();

		// TODO: Render

		// TODO: Read key states
	}

	return 0;
}