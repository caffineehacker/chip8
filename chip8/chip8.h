#pragma once

#include <string>

class chip8
{
public:
	chip8();
	~chip8();

	bool LoadProgram(const char* path);
	void StepEmulation();

private:
	unsigned char memory[4096]; // 4K memory
	unsigned char v[16]; // Vx registers
	unsigned short I; // Index register
	unsigned short pc; // Program counter
	unsigned short stack[16];
	unsigned sp;

	// Pixels on the screen, TODO: Move to a graphics class
	unsigned char gfx[64 * 32];

	// TODO: Move to a timer class
	unsigned char delay_timer;
	unsigned char sound_timer;

	// The state of the keys on the keyboard
	unsigned char key[16];
};

