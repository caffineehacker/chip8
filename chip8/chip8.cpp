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

chip8::chip8() : pc(0x200), delay_timer(0), sound_timer(0), I(0), sp(0), waitForKeypress(false) {
    memcpy_s(this->memory, sizeof(this->memory), chip8_fontset, sizeof(chip8_fontset));
    memset(this->gfx, 0, sizeof(this->gfx));
    memset(this->v, 0, sizeof(this->v));
    memset(this->key, 0, sizeof(this->key));

    // Seed the random generator
    srand(time(nullptr));
}


chip8::~chip8() {
}

bool chip8::LoadProgram(const char* path) {
    FILE* program = std::fopen(path, "rb");
    if (program == nullptr) {
        return false;
    }

    size_t readCount = fread_s(this->memory + 0x200, sizeof(this->memory) - (0x200 * sizeof(unsigned char)), sizeof(unsigned char), sizeof(this->memory) / sizeof(unsigned char) - 0x200, program);

    if (readCount < 0) {
        return false;
    }

    return true;
}

void chip8::SetKey(int keyNumber, bool state) {
    key[keyNumber] = state ? 1 : 0;
    if (state) {
        waitForKeypress = false;
    }
}

void chip8::StepEmulation() {
    needsRender = false;

    if (waitForKeypress) {
        return;
    }

    // TODO: Consider doing a fixed time per frame other than 1
    if (this->delay_timer > 0) {
        this->delay_timer--;
    }

    if (this->sound_timer > 0) {
        this->sound_timer--;

        if (this->sound_timer == 0) {
            // TODO: BEEP!
        }
    }

    // Read opcode
    short opcode = this->memory[this->pc] << 8 | this->memory[this->pc + 1];
    this->pc += 2;

    unsigned char firstNibble = static_cast<unsigned char>((opcode & 0xF000) >> 12);

    switch (firstNibble) {
        case 0x0:
        {
            if (opcode == 0x00E0) {
                // Clear the screen
                memset(this->gfx, 0, sizeof(this->gfx));
            } else if (opcode == 0x00EE) {
                // Return
                this->pc = this->stack[--this->sp];
                break;
            } else {
                throw std::logic_error("Unrecognized opcode");
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
            if (this->v[(opcode & 0x0F00) >> 8] == (opcode & 0xFF)) {
                this->pc += 2;
            }
            break;
        }
        case 0x4:
        {
            // 4XNN
            // Skips the next instruction if VX doesn't equal NN
            if (this->v[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF)) {
                this->pc += 2;
            }
            break;
        }
        case 0x5:
        {
            // 5XY0
            // 	Skips the next instruction if VX equals VY
            if (this->v[(opcode & 0x0F00) >> 8] == this->v[(opcode & 0x00F0) >> 4]) {
                this->pc += 2;
            }
            break;
        }
        case 0x6:
        {
            // 6XNN
            // Sets VX to NN
            this->v[(opcode & 0x0F00) >> 8] = static_cast<unsigned char>(opcode & 0xFF);
            break;
        }
        case 0x7:
        {
            // 7XNN
            // Adds NN to VX
            this->v[(opcode & 0x0F00) >> 8] += static_cast<unsigned char>(opcode & 0xFF);
            break;
        }
        case 0x8:
        {
            unsigned char finalNibble = opcode & 0x000F;
            switch (finalNibble) {
                case 0x0:
                {
                    // 8XY0
                    // Sets VX to the value of VY
                    this->v[(opcode & 0x0F00) >> 8] = this->v[(opcode & 0x00F0) >> 4];
                    break;
                }
                case 0x1:
                {
                    // 8XY1
                    // Sets VX to VX or VY
                    this->v[(opcode & 0x0F00) >> 8] |= this->v[(opcode & 0x00F0) >> 4];
                    break;
                }
                case 0x2:
                {
                    // 8XY2
                    // Sets VX to VX and VY
                    this->v[(opcode & 0x0F00) >> 8] &= this->v[(opcode & 0x00F0) >> 4];
                    break;
                }
                case 0x3:
                {
                    // 8XY3
                    // Sets VX to VX xor VY
                    this->v[(opcode & 0x0F00) >> 8] ^= this->v[(opcode & 0x00F0) >> 4];
                    break;
                }
                case 0x4:
                {
                    // 8XY4
                    // Adds VY to VX. VF is set to 1 when there's a carry and to 0 when there isn't
                    unsigned char vy = this->v[(opcode & 0x00F0) >> 4];
                    unsigned char vx = this->v[(opcode & 0x0F00) >> 8];
                    this->v[(opcode & 0x0F00) >> 8] += vy;
                    // Set the carry flag
                    this->v[0xF] = (vx + vy) > 0xFF ? 1 : 0;
                    break;
                }
                case 0x5:
                {
                    // 8XY5
                    /* VY is subtracted from VX. VF is set to 0 when there's a borrow and to 0 when there
                     * isn't
                     */
                    unsigned char vy = this->v[(opcode & 0x00F0) >> 4];
                    unsigned char vx = this->v[(opcode & 0x0F00) >> 8];
                    this->v[(opcode & 0x0F00) >> 8] -= vy;
                    // Set the carry flag
                    this->v[0xF] = (vx - vy) < 0 ? 1 : 0;
                    break;
                }
                case 0x6:
                {
                    // 8XY6
                    /* Shifts VX right by one. VF is set to the value of the least significant bit of VX
                     * before the shift
                     */
                    this->v[0xF] = this->v[(opcode & 0x0F00) >> 8] & 0x01;
                    this->v[(opcode & 0x0F00) >> 8] >>= 1;
                    break;
                }
                case 0xE:
                {
                    // 8XYE
                    /* Shifts VX left by one. VF is set to the value of the most significant bit of VX
                     * before the shift
                     */
                    this->v[0xF] = (this->v[(opcode & 0x0F00) >> 8] & 0x80) >> 15;
                    this->v[(opcode & 0x0F00) >> 8] <<= 1;
                    break;
                }
                default:
                    throw std::logic_error("Unrecognized opcode");
            }
            break;
        }
        case 0x9:
        {
            // 9XY0
            // Skips the next instruction if VX doesn't equals VY
            if (this->v[(opcode & 0x0F00) >> 8] != this->v[(opcode & 0x00F0) >> 4]) {
                this->pc += 2;
            }
            break;
        }
        case 0xA:
        {
            // ANNN
            // Sets I to the address NNN
            this->I = (opcode & 0x0FFF);
            break;
        }
        case 0xB:
        {
            throw std::logic_error("Unrecognized opcode");
        }
        case 0xC:
        {
            // CXNN
            // Sets VX to the result of a bitwise and operation on a random number and NN
            this->v[(opcode & 0x0F00) >> 8] = (opcode & 0xFF) & (rand() % 0x100);
            break;
        }
        case 0xD:
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

            for (int row = 0; row < rows; row++) {
                unsigned char pixelRow = this->memory[this->I + row];
                for (int x = startX; x < startX + 4; x++) {
                    if ((((pixelRow >> 4) & (0x0001 << (3 - (x - startX)))) >> (3 - (x - startX))) == 0) {
                        // Pixel is empty and we draw in XOR mode
                        continue;
                    }

                    int actualX = x % 64; // Screen wrap
                    int actualY = (startY + row) % 32;

                    if (this->gfx[actualX + (actualY * 64)] == 1) {
                        this->v[0xF] = 1;
                    }

                    this->gfx[actualX + (actualY * 64)] ^= 0x1;

                    needsRender = true;
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
            if ((this->key[v[(opcode & 0x0F00) >> 8]] != 0) == (((opcode & 0xFF) == 0x9E))) {
                this->pc += 2;
            }

            if ((opcode & 0xFF) != 0x9E && (opcode & 0xFF) != 0xA1) {
                throw std::logic_error("Unrecognized opcode");
            }

            break;
        }
        case 0xF:
        {
            unsigned char secondByte = static_cast<unsigned char>(opcode & 0xFF);
            unsigned char secondNibble = static_cast<unsigned char>((opcode & 0x0F00) >> 8);
            switch (secondByte) {
                case 0x07:
                {
                    this->v[secondNibble] = this->delay_timer;
                    break;
                }
                case 0x0A:
                {
                    // Wait for any key press
                    waitForKeypress = true;
                    return;
                }
                case 0x15:
                {
                    this->delay_timer = this->v[secondNibble];
                    break;
                }
                case 0x18:
                {
                    // Sets the sound timer to VX
                    this->sound_timer = this->v[secondNibble];
                    break;
                }
                case 0x1E:
                {
                    // Adds VX to I
                    this->I += this->v[secondNibble];
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
                case 0x55:
                {
                    // FX55
                    // Stores V0 to VX (including VX) in memory starting at address I
                    // NOTE: Modern interpreters don't change I, orginial ones did
                    for (int i = 0; i <= secondNibble; i++) {
                        this->memory[this->I + i] = this->v[i];
                    }
                    break;
                }
                case 0x65:
                {
                    // Fills V0 to VX (including VX) with values from memory starting at address I
                    // NOTE: Modern interpreters don't change I, orginial ones did
                    for (int i = 0; i <= secondNibble; i++) {
                        this->v[i] = this->memory[this->I + i];
                    }
                    break;
                }
                default:
                {
                    throw std::logic_error("Unrecognized opcode");
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