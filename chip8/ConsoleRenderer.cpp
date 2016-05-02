#include "ConsoleRenderer.h"

#include <exception>
#include <string>
#include <Windows.h>

ConsoleRenderer::ConsoleRenderer()
{
	this->hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	this->hBuffer = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, CONSOLE_TEXTMODE_BUFFER, nullptr);
	SetConsoleActiveScreenBuffer(this->hBuffer);
}


ConsoleRenderer::~ConsoleRenderer()
{
}

void ConsoleRenderer::Render(const unsigned char* buffer)
{
	CHAR_INFO char_info[64 * 32];
	for (int i = 0; i < 32*64; i++)
	{
		char_info[i].Char.AsciiChar = L'A';
		char_info[i].Attributes = buffer[i] >= 1 ? 0xFF : 0x00;
	}

	SMALL_RECT output_rect{ 0, 0, 64, 32 };
	if (0 == WriteConsoleOutput(this->hBuffer, char_info, { 64, 32 }, { 0, 0 }, &output_rect))
	{
		DWORD error = GetLastError();
		throw std::exception((std::string("Drawing failed") + itoa(error, nullptr, 10)).c_str());
	}
}