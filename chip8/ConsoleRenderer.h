#pragma once
#include <Windows.h>

class ConsoleRenderer
{
public:
	ConsoleRenderer();
	~ConsoleRenderer();

	void Render(const unsigned char* buffer);

private:
	HANDLE hConsole;
	HANDLE hBuffer1;
	HANDLE hBuffer2;
};

