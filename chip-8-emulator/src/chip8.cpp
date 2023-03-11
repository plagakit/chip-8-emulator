#include "chip8.h"

#include <iostream>
#include <stdexcept>

using BYTE = unsigned char;
using WORD = short;

/*

CHIP-8 Memory Map:

	000 - 04F	Empty space
	050 - 09F	Font data
	100 - 1FF	Empty space
	200 - FFF	CHIP-8 Game

*/
BYTE RAM[4096];

bool display[64][32];

WORD PC; // program counter

WORD I; // index register

WORD stack[16];

BYTE delayTimer;

BYTE soundTimer;

BYTE V[16]; // variable registers 0 - F


/*

Example of how a character is represented in font:

"0"	Binary	Hex
****	11110000	0xF0
*  *	10010000	0x90
*  *	10010000	0x90
*  *	10010000	0x90
****	11110000	0xF0

*/
BYTE font[] = {
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


void Init()
{
	PC = 0x0200; // first thing in data
	I = 0x0000;
	
	std::cout << "Initialzied CHIP-8." << std::endl;
}

void LoadFile(const char* path)
{
	FILE* file = fopen(path, "rb");	
	if (file == NULL)
	{
		std::cout << "Error: File " + std::string(path) + " not found.\n";
		exit(1);
	}
	
	// Load file data into buffer
	const int gameSize = 0xFFF - 0x1FF; // max size of game (see memory map)
	BYTE buffer[gameSize];
	fread(buffer, sizeof(buffer), 1, file);

	// Print out bytes
	std::cout << "Reading memory from " << std::string(path) << "...\n";
	for (int i = 0; i < gameSize; i++)
		printf("%02x ", buffer[i]);
	std::cout << std::endl;

	// Load the game into RAM
	for (int i = 0; i < gameSize; i++)
		RAM[0x200 + i] = buffer[i];

}

void Update()
{
	// Fetch

	BYTE b1 = RAM[PC];		// byte 1
	BYTE b2 = RAM[PC + 1];	// byte 2

	BYTE instruction = b1 >> 4;	// first nibble
	BYTE vx = b1 & 0x0F;		// second nibble (x register in V)
	BYTE vy = b2 >> 4;			// third nibble (y register in V)
	BYTE n = b2 & 0x0F;			// fourth nibble

	PC += 2; // move program counter to next instruction

	printf("b1: %02x\tb2: %02x\tinst: %02x\n", b1, b2, instruction);

}