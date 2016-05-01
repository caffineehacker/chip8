#include "chip8.h"

#include <cstdio>
#include <cstdlib>
#include <ctime>

const unsigned char chip8_fontset[80] =
{
	0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
	0x20, 0x60, 0x20, 0x20, 0x70, // 1
	0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
	0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
	0x90, 0x90, 0xF0, 0x10, 0x10, // 4
	0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
	0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
	0xF0, 0x10, 0x20, 0x40, 0x40, // 7
	0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
	0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
	0xF0, 0x90, 0xF0, 0x90, 0x90, // A
	0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
	0xF0, 0x80, 0x80, 0x80, 0xF0, // C
	0xE0, 0x90, 0x90, 0x90, 0xE0, // D
	0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
	0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

chip8::chip8() : pc(0x200), delay_timer(0), sound_timer(0), I(0), sp(0)
{
	memcpy_s(this->memory, sizeof(this->memory), chip8_fontset, sizeof(chip8_fontset));
	memset(this->gfx, 0, sizeof(this->gfx));
	memset(this->v, 0, sizeof(this->v));
	memset(this->key, 0, sizeof(this->key));

	// Seed the random generator
	srand(time(nullptr));
}


chip8::~chip8()
{
}

bool chip8::LoadProgram(const char* path)
{
	FILE* program = std::fopen(path, "rb");
	if (program == nullptr)
	{
		return false;
	}

	size_t readCount = fread_s(this->memory + 0x200, sizeof(this->memory) - (0x200 * sizeof(unsigned char)), sizeof(unsigned char), sizeof(this->memory) / sizeof(unsigned char) - 0x200, program);

	if (readCount < 0)
	{
		return false;
	}

	return true;
}

void chip8::StepEmulation()
{
	// TODO: Consider doing a fixed time per frame other than 1
	if (this->delay_timer > 0)
	{
		this->delay_timer--;
	}

	if (this->sound_timer > 0)
	{
		this->sound_timer--;

		if (this->sound_timer == 0)
		{
			// TODO: BEEP!
		}
	}

	// Read opcode
	short opcode = this->memory[this->pc] << 8 | this->memory[this->pc + 1];
	this->pc += 2;

	unsigned char firstNibble = static_cast<unsigned char>((opcode & 0xF000) >> 12);

	switch (firstNibble)
	{
	case 0x0:
	{
		if (opcode == 0x00EE)
		{
			// Return
			this->pc = this->stack[--this->sp];
			break;
		}
		break;
	}
	case 0x1:
	{
		// 1NNN
		// Jumps to NNN
		this->pc = opcode & 0x0FFF;
		break;
	}
	case 0x2:
	{
		// 2NNN
		// Calls subroutine at NNN
		this->stack[this->sp++] = this->pc;
		this->pc = opcode & 0x0FFF;
		break;
	}
	case 0x3:
	{
		// 3XNN
		// Skips the next instruction if VX == NN
		if (this->v[(opcode & 0x0F00) >> 16] == (opcode & 0xFF))
		{
			this->pc += 2;
		}
		break;
	}
	case 0x6:
	{
		// 6XNN
		// Sets VX to NN
		this->v[(opcode & 0x0F00) >> 16] = static_cast<unsigned char>(opcode & 0xFF);
		break;
	}
	case 0x7:
	{
		// 7XNN
		// Adds NN to VX
		this->v[(opcode & 0x0F00) >> 16] += static_cast<unsigned char>(opcode & 0xFF);
		break;
	}
	case 0xa:
	{
		// ANNN
		// Sets I to the address NNN
		this->I = (opcode & 0x0FFF);
		break;
	}
	case 0xc:
	{
		// CXNN
		// Sets VX to the result of a bitwise and operation on a random number and NN
		this->v[(opcode & 0x0F00) >> 16] = (opcode & 0xFF) & (rand() % 0x100);
		break;
	}
	case 0xd:
	{
		// DXYN
		/* From Wikipedia:
		 * Sprites stored in memory at location in index register (I), 8bits wide. Wraps around
		 * the screen. If when drawn, clears a pixel, register VF is set to 1 otherwise it is zero.
		 * All drawing is XOR drawing (i.e. it toggles the screen pixels). Sprites are drawn
		 * starting at position VX, VY. N is the number of 8bit rows that need to be drawn. If N is
		 * greater than 1, second line continues at position VX, VY+1, and so on.
         */
		this->v[0xF] = 0; // Default to 0

		unsigned char startX = this->v[(opcode & 0x0F00) >> 8];
		unsigned char startY = this->v[(opcode & 0x00F0) >> 4];
		unsigned char rows = opcode & 0x000F;

		for (int row = 0; row < rows; row++)
		{
			unsigned char pixelRow = this->memory[this->I + row];
			for (int x = startX; x < 8; x++)
			{
				if (((pixelRow & (0x0001 << x)) >> x) == 0)
				{
					// Pixel is empty and we draw in XOR mode
					continue;
				}

				int actualX = x % 64; // Screen wrap
				int actualY = (startY + row) % 32;

				if (this->gfx[actualX + (actualY * 64)] == 1)
				{
					this->v[0xF] = 1;
				}

				this->gfx[actualX + (actualY * 64)] ^= 0x1;
			}
		}
		break;
	}
	case 0xE:
	{
		// EX9E
		// Skips the next instruction if the key stored in VX is pressed.
		// EXA1
		// Skips the next instruction if the key stored in VX isn't pressed.
		if (this->key[(opcode & 0x0F00) >> 16] == ((opcode & 0xFF) == 0x9E))
		{
			this->pc += 2;
		}

#if _DEBUG
		if ((opcode & 0xFF) != 0x9E && (opcode & 0xFF) != 0xA1)
		{
			throw std::exception("Invalid opcode");
		}
#endif

		break;
	}
	case 0xF:
	{
		unsigned char secondByte = static_cast<unsigned char>(opcode & 0xFF);
		unsigned char secondNibble = static_cast<unsigned char>((opcode & 0x0F00) >> 8);
		switch (secondByte)
		{
		case 0x07:
		{
			this->v[secondNibble] = this->delay_timer;
			break;
		}
		case 0x15:
		{
			this->delay_timer = this->v[secondNibble];
			break;
		}
		case 0x29:
		{
			/* From Wikipedia: Sets I to the location of the sprite for the character in VX.
			 * Characters 0-F (in hexadecimal) are represented by a 4x5 font.
			 */
			this->I = 5 * this->v[secondNibble];
			break;
		}
		case 0x33:
		{
			/* From Wikipedia: Stores the binary-coded decimal representation of VX, with the most
			 * significant of three digits at the address in I, the middle digit at I plus 1, and
			 * the least significant digit at I plus 2. (In other words, take the decimal
			 * representation of VX, place the hundreds digit in memory at location in I, the tens
			 * digit at location I+1, and the ones digit at location I+2.)
			 */
			unsigned short value = this->v[secondNibble];
			this->memory[this->I] = value / 100;
			this->memory[this->I + 1] = (value / 10) % 10;
			this->memory[this->I + 2] = value % 10;
			break;
		}
		case 0x65:
		{
			// Fills V0 to VX (including VX) with values from memory starting at address I
			// NOTE: Modern interpreters don't change I, orginial ones did
			for (int i = 0; i <= secondNibble; i++)
			{
				this->v[i] = this->memory[this->I + i];
			}
			break;
		}
		default:
		{
			throw std::exception("Unknown opcode");
		}
		}
		break;
	}
	default:
	{
		throw std::exception("Unknown opcode");
	}
	}
}