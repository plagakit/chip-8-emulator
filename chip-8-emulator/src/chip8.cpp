#include "chip8.h"

#include <iostream>
#include <stdexcept>
#include <bitset>


CHIP8::CHIP8(const char* path)
{
	// Read ROM

	FILE* file = fopen(path, "rb");
	if (file == NULL)
	{
		std::cout << "Error: File " + std::string(path) + " not found.\n";
		exit(1);
	}
	std::cout << "Reading memory from " << std::string(path) << "...\n";

	const int gameSize = 0x1000 - 0x200; // max size of game (see memory map)
	BYTE buffer[gameSize];
	fread(buffer, sizeof(buffer), 1, file);


	// Load ROM & font into RAM

	for (int i = 0; i < gameSize; i++)
		RAM[0x200 + i] = buffer[i];

	for (int i = 0; i < sizeof(font) / sizeof(font[0]); i++)
		RAM[0x050 + i] = font[i];


	// Initialize members
	
	I = 0x000;
	PC = 0x0200; // pointer to start of ROM in RAM
	delayTimer = 0x00;
	soundTimer = 0x00;

	for (int i = 0; i < 16; i++)
	{
		V[i] = 0x00;
		stack[i] = 0x0000;
	}

	for (int i = 0; i < 32; i++)
		for (int j = 0; j < 64; j++)
			display[i][j] = false;


	std::cout << "Initialized CHIP-8." << std::endl;
}

void CHIP8::Update()
{
	// Fetch

	BYTE b1 = RAM[PC];		// byte 1
	BYTE b2 = RAM[PC + 1];	// byte 2, denoted by nn

	WORD opcode = (b1 << 8) | b2;	// concat 2 bytes
	WORD addr = opcode & 0x0FFF;	// nnn, lower 12 bits
	BYTE x = b1 & 0x0F;			// second nibble (x register in V)
	BYTE y = b2 >> 4;			// third nibble (y register in V)
	BYTE n = b2 & 0x0F;			// fourth nibble, last 4 bits

	PC += 2; // move program counter to next instruction

	printf("Running %04x: ", opcode);

	// 00E0 - CLS
	// Clear the screen.
	if (opcode == 0x00E0)
	{
		printf("Clear screen");
		for (int i = 0; i < 32; i++)
			for (int j = 0; j < 64; j++)
				display[i][j] = 0;
	}

	// 1NNN - JMP addr
	// Sets PC to addr.
	else if ((b1 >> 4) == 1)
	{
		printf("Jump to %03x", addr);
		PC = addr;
	}

	// 6XNN - LD Vx, byte
	// Set xth register in V to byte
	else if ((b1 >> 4) == 6)
	{
		printf("Set V[%d] to %02x", x, b2);
		V[x] = b2;
	}

	// 7XNN - ADD Vx, byte
	// Add byte to register x
	else if ((b1 >> 4) == 7)
	{
		printf("Add V[%d] with %02x", x, b2);
		V[x] = V[x] + b2;
	}

	// ANNN - LD I, addr
	// Set index register to addr
	else if ((b1 >> 4) == 0xA)
	{
		printf("Set I reg to %03x", addr);
		I = addr;
	}

	// DXYN - DRW Vx, Vy, nibble
	// Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision.
	else if ((b1 >> 4) == 0xD)
	{
		BYTE px = V[x];// % 64;
		BYTE py = V[y];// % 32;
		V[0xF] = 0; // set collision to false

		printf("Draw (I = %03x)", I);

		for (int i = 0; i < n; i++)
		{
			if (py + i >= 32)
				break;

			BYTE row = RAM[I + i];
			//printf("Row %d: %x,\t", i, row);

			for (int j = 0; j < 8; j++)
			{
				if (px + j >= 64)
					break;

				bool pixel = display[py + i][px + j];
				bool sprite = (row & (128 >> j)) != 0; // if jth bit in byte is on

				//printf("%c", sprite ? '#' : '.');

				if (pixel && sprite)
				{
					display[py + i][px + j] = false;
					V[0xF] = 1;
				}
				else if (sprite)
					display[py + i][px + j] = true;
			
				//printf("%d", sprite);
			}

			//printf("\n");
		}
	}

	printf("\n");
}