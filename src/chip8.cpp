﻿#include "chip8.h"

#include <iostream>
#define FMT_HEADER_ONLY
#include <fmt/core.h>

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

	InitFromBuffer(buffer);
}

void CHIP8::InitFromBuffer(BYTE* rom)
{
	// Load ROM & font into RAM
	for (int i = 0; i < 0xE00; i++)
		RAM[0x200 + i] = rom[i];

	for (int i = 0; i < sizeof(font) / sizeof(font[0]); i++)
		RAM[0x050 + i] = font[i];


	// Initialize members
	I = 0x000;
	PC = 0x0200; // pointer to start of ROM in RAM
	DT = 0x00;
	ST = 0x00;

	for (int i = 0; i < 16; i++)
	{
		V[i] = 0x00;
		keyStates[i] = false;
	}

	for (int i = 0; i < 32; i++)
		for (int j = 0; j < 64; j++)
			display[i][j] = false;


	std::cout << "Initialized CHIP-8." << std::endl;
}


void CHIP8::Cycle()
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

	std::string instStr = fmt::format("> {:04X} - ", opcode);

	// 00E0 - CLS
	// Clear the screen.
	if (opcode == 0x00E0)
	{
		instStr += "CLS";
		for (int i = 0; i < 32; i++)
			for (int j = 0; j < 64; j++)
				display[i][j] = 0;
	}

	// 00EE - RET
	// Pop last address on stack and set PC to it (effectively returning from a function)
	else if (opcode == 0x00EE)
	{
		WORD ret = stack.top();
		stack.pop();
		PC = ret;
		instStr += "RET";//std::format("Pop last address on stack and set PC to {:04X}", ret);
	}

	// 1NNN - JMP addr
	// Sets PC to addr.
	else if ((b1 >> 4) == 1)
	{
		instStr += fmt::format("JMP {:04X}", addr);
		PC = addr;
	}

	// 2NNN - CALL addr
	// Puts current PC on stack then sets PC to addr, (effectively calls a function at addr)
	else if ((b1 >> 4) == 2)
	{
		instStr += fmt::format("CALL {:04X}", PC);
		stack.push(PC);
		PC = addr;
	}

	// 3XNN - SE Vx, byte
	// Skips next instruction if Vx = nn
	else if ((b1 >> 4) == 3)
	{
		instStr += fmt::format("SE V[{:01X}] ({:02X}), {:02X}", x, V[x], b2);
		if (V[x] == b2)
			PC += 2;
	}

	// 4XNN - SNE Vx, byte
	// Skips next instruction if Vx != nn
	else if ((b1 >> 4) == 4)
	{
		instStr += fmt::format("SNE V[{:01X}] ({:02X}), {:02X}", x, V[x], b2);
		if (V[x] != b2)
			PC += 2;
	}

	// 5XY0 - SE Vx, Vy
	// Skips next instruction if Vx = Vy
	else if ((b1 >> 4) == 5)
	{
		instStr += fmt::format("SE V[{:01X}] ({:02X}), V[{:01X}] ({:02X})", x, V[x], y, V[y]);
		if (V[x] == V[y])
			PC += 2;
	}

	// 6XNN - LD Vx, byte
	// Set xth register in V to byte
	else if ((b1 >> 4) == 6)
	{
		instStr += fmt::format("LD V[{:01X}], {:02X}", x, b2);
		V[x] = b2;
	}

	// 7XNN - ADD Vx, byte
	// Add byte to register x
	else if ((b1 >> 4) == 7)
	{
		instStr += fmt::format("ADD V[{:01X}], {:02X}", x, b2);
		V[x] = V[x] + b2;
	}

	// 9XY0 - SNE Vx, Vy
	// Skips next instruction if Vx != Vy
	else if ((b1 >> 4) == 9)
	{
		instStr += fmt::format("SNE V[{:01X}] ({:02X}), V[{:01X}] ({:02X})", x, V[x], y, V[y]);
		if (V[x] != V[y])
			PC += 2;
	}

	// 8___ - Logical & arithmetic instuctions
	else if ((b1 >> 4) == 8)
	{
		// 8XY0 - LD Vx, Vy
		// Set Vx = Vy
		if (n == 0)
		{
			instStr += fmt::format("LD V[{:01X}], V[{:01X}]", x, y);
			V[x] = V[y];
		}

		// 8XY1 - OR Vx, Vy
		// Set Vx = Vx OR Vy
		else if (n == 1)
		{
			instStr += fmt::format("OR V[{:01X}], V[{:01X}]", x, y);
			V[x] = V[x] | V[y];
		}

		// 8XY2 - AND Vx, Vy
		// Set Vx = Vx AND Vy
		else if (n == 2)
		{
			instStr += fmt::format("AND V[{:01X}], V[{:01X}]", x, y);
			V[x] = V[x] & V[y];
		}

		// 8XY3 - XOR Vx, Vy
		// Set Vx = Vx XOR Vy
		else if (n == 3)
		{
			instStr += fmt::format("XOR V[{:01X}], V[{:01X}]", x, y);
			V[x] = V[x] ^ V[y];
		}

		// 8XY4 - ADD Vx, Vy
		// Set Vx = Vx + Vy, set VF = carry bit if overflowed
		else if (n == 4)
		{
			instStr += fmt::format("ADD V[{:01X}], V[{:01X}]", x, y);
			int overflow = (int)(V[x]) + (int)(V[y]) > 0xFF;
			V[x] = V[x] + V[y];
			V[0xF] = overflow;
		}

		// 8XY5 - SUB Vx, Vy
		// Set Vx =	Vx - Vy, set Vf = NOT borrow (if Vx >= Vy)
		else if (n == 5)
		{
			instStr += fmt::format("SUB V[{:01X}], V[{:01X}]", x, y);
			int borrow = !(V[x] < V[y]);
			V[x] = V[x] - V[y];
			V[0xF] = borrow;
		}

		// 8XY6 - SHR Vx
		// (Ambiguous) Shift Vx right by 1, set Vf = shifted out bit
		else if (n == 6)
		{
			instStr += fmt::format("SHR V[{:01X}]", x);
			V[0xF] = V[x] & 1; // rightmost bit
			V[x] >>= 1;
		}

		// 8XY7 - SUBN Vx, Vy
		// Set Vx = Vy - Vx, set Vf = NOT borrow (if Vy > Vx)
		else if (n == 7)
		{
			instStr += fmt::format("SUBN V[{:01X}], V[{:01X}]", y, x);
			V[x] = V[y] - V[x];
			V[0xF] = V[y] > V[x];
		}

		// 8XYE - SHL Vx
		// (Ambiguous) Shift Vx left by 1, set Vf = shifted out bit
		else if (n == 0xE)
		{
			instStr += fmt::format("SHL V[{:01X}]", x);
			V[0xF] = V[x] & 128; // leftmost bit in byte
			V[x] <<= 1;
		}
	}

	// ANNN - LD I, addr
	// Set index register to addr
	else if ((b1 >> 4) == 0xA)
	{
		instStr += fmt::format("LD I, {:04X}", addr);
		I = addr;
	}

	// BNNN - JMP V0, addr
	// (Ambiguous, went with old CHIP-8) Program counter is set to addr plus V0
	else if ((b1 >> 4) == 0xB)
	{
		instStr += fmt::format("JMP V0, {:04X}", addr);
		PC = addr + V[0];
	}

	// CXNN - RND Vx, byte
	// Set Vx = random byte AND nn
	else if ((b1 >> 4) == 0xC)
	{
		instStr += fmt::format("RND V[{:01X}], {:02X}", x, b2);
		
		static std::random_device rd;
		static std::mt19937 rng(rd());
		std::uniform_int_distribution<> randByte(0, 256);
		
		V[x] = randByte(rng) & b2;
	}

	// DXYN - DRW Vx, Vy, nibble
	// Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision.
	else if ((b1 >> 4) == 0xD)
	{
		BYTE px = V[x];// % 64;
		BYTE py = V[y];// % 32;
		V[0xF] = 0; // set collision to false

		instStr += fmt::format("DRW V[{:01X}], V[{:01X}], {:02X}", x, y, n);

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

	// EX9E - SKP Vx
	// Skip next instruction if key w/ value of Vx is pressed
	else if ((b1 >> 4) == 0xE && b2 == 0x9E)
	{
		instStr += fmt::format("SKP V[{:01X}]", x);
		if (keyStates[V[x]])
			PC += 2;
	}

	// EXA1 - SKNP Vx
	// Skip next instruction if key w/ value of Vx is NOT pressed
	else if ((b1 >> 4) == 0xE && b2 == 0xA1)
	{
		instStr += fmt::format("SKNP V[{:01X}]", x);
		if (!keyStates[V[x]])
			PC += 2;
	}

	// FX07 - LD Vx, DT
	// Set Vx to the current value of the delay timer
	else if ((b1 >> 4) == 0xF && b2 == 0x07)
	{
		instStr += fmt::format("LD V[{:01X}], DT", x);
		V[x] = DT;
	}

	// FX0A - LD Vx, K
	// Blocks instructions until a key is pressed, key stored in Vx
	else if ((b1 >> 4) == 0xF && b2 == 0x0A)
	{
		instStr += fmt::format("BLK V[{:01X}", x);
		if (!keyStates[V[x]])
			PC -= 2;
	}

	// FX15 - LD DT, Vx
	// Set delay timer to value in Vx
	else if ((b1 >> 4) == 0xF && b2 == 0x15)
	{
		instStr += fmt::format("LD DT, V[{:01X}]", x);
		DT = V[x];
	}

	// FX18 - LD ST, Vx
	// Set sound timer to value in Vx
	else if ((b1 >> 4) == 0xF && b2 == 0x18)
	{
		instStr += fmt::format("LD ST, V[{:01X}]", x);
		ST = V[x];
	}

	// FX1E - ADD I, Vx
	// Set I = I + Vx, set Vf = overflow
	else if ((b1 >> 4) == 0xF && b2 == 0x1E)
	{
		instStr += fmt::format("ADD I, V[{:01X}]", x);
		I = I + V[x];
		V[0xF] = (int)I + (int)V[x] > 0xFFF;
	}

	// FX29 - LD F, Vx
	// Set I = location of font sprite for digit Vx
	else if ((b1 >> 4) == 0xF && b2 == 0x29)
	{
		instStr += fmt::format("LD I, font of V[{:01X}]", x);
		I = 0x50 + V[x] * 6; // font sprite is 6 bytes
	}

	// FX33 - LD B, Vx
	// Stores the 3 digits of the integer representation of Vx @ I, I+1 and I+2
	else if ((b1 >> 4) == 0xF && b2 == 0x33)
	{
		instStr += fmt::format("LD I, digits of V[{:01X}]", x);
		RAM[I] = (V[x] / 100) % 10;
		RAM[I + 1] = (V[x] / 10) % 10;
		RAM[I + 2] = V[x] % 10;
	}

	// FX55 - LD [I], Vx
	// (Ambiguous) Stores values from V0-Vx inclusive @ I-I+x (without changing I)
	else if ((b1 >> 4) == 0xF && b2 == 0x55)
	{
		//printf("Store V0-V%x to I\n", V[x]);
		instStr += fmt::format("LD [I], V0-V[{:01X}]", x);
		for (int i = 0; i <= x; i++)
			RAM[I + i] = V[i];
	}

	// FX65 - LD Vx, [I]
	// (Ambiguous) Loads values from I-I+x into V0-Vx inclusive (without changing I)
	else if ((b1 >> 4) == 0xF && b2 == 0x65)
	{
		instStr += fmt::format("LD V0-V[{:01X}], [I]", x);
		for (int i = 0; i <= x; i++)
			V[i] = RAM[I + i];
	}

	else
	{
		instStr += fmt::format("UNKNOWN");
	}

	//printf("\n");

	instructionList.push_front(instStr);
	if (instructionList.size() > 100)
		instructionList.pop_back();
}