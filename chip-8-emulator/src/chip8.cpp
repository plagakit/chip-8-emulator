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

BYTE display[64][32];

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
	for (int i = 0; i < 64; i++)
		for (int j = 0; j < 32; j++)
			display[i][j] = 0;
	
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
	//for (int i = 0; i < gameSize; i++)
	//	printf("%02x ", buffer[i]);
	//std::cout << std::endl;

	// Load the game into RAM
	for (int i = 0; i < gameSize; i++)
		RAM[0x200 + i] = buffer[i];

}

void Update()
{
	// Fetch

	BYTE b1 = RAM[PC];		// byte 1
	BYTE b2 = RAM[PC + 1];	// byte 2, denoted by nn
	
	WORD inst = (b1 << 8) | b2;	// concat 2 bytes
	WORD addr = inst & 0x0FFF;	// nnn, lower 12 bits
	BYTE x = b1 & 0x0F;					// second nibble (x register in V)
	BYTE y = b2 >> 4;					// third nibble (y register in V)
	BYTE n = b2 & 0x0F;					// fourth nibble, last 4 bits

	PC += 2; // move program counter to next instruction

	// 00E0 - CLS
	// Clear the screen.
	if (inst == 0x00E0)
	{
		for (int i = 0; i < 64; i++)
			for (int j = 0; j < 32; j++)
				display[i][j] = 0;
	}

	// 1NNN - JMP addr
	// Sets PC to addr.
	else if ((b1 >> 4) == 1)
	{
		PC = addr;
	}

	// 6XNN - LD Vx, byte
	// Set xth register in V to byte
	else if ((b1 >> 4) == 6)
	{
		V[x] = b2;
	}

	// 7XNN - ADD Vx, byte
	// Add byte to register x
	else if ((b1 >> 4) == 7)
	{
		V[x] = V[x] + b2;
	}

	// ANNN - LD I, addr
	// Set index register to addr
	else if ((b1 >> 4) == 0xA)
	{
		I = b2;
	}

	// DXYN - DRW Vx, Vy, nibble
	// Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision.
	else if ((b1 >> 4) == 0xD)
	{
		V[0xF] = 0; // set collision to false
		for (int i = 0; i < n && V[y]+i < 32; i++)
		{
			BYTE row = RAM[I + i];
			BYTE ycoord = V[y] + i;
			
			for (int j = 0; j < 8 && V[x]+j < 64; j++)
			{
				BYTE xcoord = V[x] + j;
				BYTE bit = (row & (1 << j)) >> j;
				
				display[xcoord][ycoord] = display[xcoord][ycoord] ^ bit;
				V[0xF] = V[0xF] | (display[xcoord][ycoord] & bit); // set if theres a collision
			}

		}
	}
}