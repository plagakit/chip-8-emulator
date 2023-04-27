#include "chip8.h"

#include <iostream>

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
		V[i] = 0x00;

	for (int i = 0; i < 32; i++)
		for (int j = 0; j < 64; j++)
			display[i][j] = false;


	std::cout << "Initialized CHIP-8." << std::endl;
}


bool CHIP8::QueryKey(BYTE key)
{
	/*switch (key)
	{
	case 0: return sf::Keyboard::isKeyPressed(sf::Keyboard::X); 
	case 1: return sf::Keyboard::isKeyPressed(sf::Keyboard::Num1);
	case 2: return sf::Keyboard::isKeyPressed(sf::Keyboard::Num2);
	case 3: return sf::Keyboard::isKeyPressed(sf::Keyboard::Num3);
	case 4: return sf::Keyboard::isKeyPressed(sf::Keyboard::Q);
	case 5: return sf::Keyboard::isKeyPressed(sf::Keyboard::W);
	case 6: return sf::Keyboard::isKeyPressed(sf::Keyboard::E);
	case 7: return sf::Keyboard::isKeyPressed(sf::Keyboard::A);
	case 8: return sf::Keyboard::isKeyPressed(sf::Keyboard::S);
	case 9: return sf::Keyboard::isKeyPressed(sf::Keyboard::D);
	case 0xA: return sf::Keyboard::isKeyPressed(sf::Keyboard::Z);
	case 0xB: return sf::Keyboard::isKeyPressed(sf::Keyboard::C);
	case 0xC: return sf::Keyboard::isKeyPressed(sf::Keyboard::Num4);
	case 0xD: return sf::Keyboard::isKeyPressed(sf::Keyboard::R);
	case 0xE: return sf::Keyboard::isKeyPressed(sf::Keyboard::F);
	case 0xF: return sf::Keyboard::isKeyPressed(sf::Keyboard::V);
	default: return false;
	}*/
	return false;
}


void CHIP8::Update()
{
	// Update timers
	// TODO: change this so it matches 60Hz
	delayTimer--;
	soundTimer--;
	
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

	// 00EE - RET
	// Pop last address on stack and set PC to it (effectively returning from a function)
	else if (opcode == 0x00EE)
	{
		WORD ret = stack.top();
		stack.pop();
		PC = ret;
		printf("Pop last address on stack and set PC to %04x", ret);
	}

	// 1NNN - JMP addr
	// Sets PC to addr.
	else if ((b1 >> 4) == 1)
	{
		printf("Jump to %03x", addr);
		PC = addr;
	}

	// 2NNN - CALL addr
	// Puts current PC on stack then sets PC to addr, (effectively calls a function at addr)
	else if ((b1 >> 4) == 2)
	{
		printf("Push %04x to stack", PC);
		stack.push(PC);
		PC = addr;
	}

	// 3XNN - SE Vx, byte
	// Skips next instruction if Vx = nn
	else if ((b1 >> 4) == 3)
	{
		printf("Skip next instruction if V[%02x] (%02x) == %02x", x, V[x], b2);
		if (V[x] == b2)
			PC += 2;
	}

	// 4XNN - SNE Vx, byte
	// Skips next instruction if Vx != nn
	else if ((b1 >> 4) == 4)
	{
		printf("Skip next instruction if V[%02x] (%02x) != %02x", x, V[x], b2);
		if (V[x] != b2)
			PC += 2;
	}

	// 5XY0 - SE Vx, Vy
	// Skips next instruction if Vx = Vy
	else if ((b1 >> 4) == 5)
	{
		printf("Skip next instruction if V[%02x] (%02x) == V[%02x] (%02x)", x, V[x], y, V[y]);
		if (V[x] == V[y])
			PC += 2;
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

	// 9XY0 - SNE Vx, Vy
	// Skips next instruction if Vx != Vy
	else if ((b1 >> 4) == 9)
	{
		printf("Skip next instruction if V[%02x] (%02x) != V[%02x] (%02x)", x, V[x], y, V[y]);
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
			V[x] = V[y];
		}

		// 8XY1 - OR Vx, Vy
		// Set Vx = Vx OR Vy
		else if (n == 1)
		{
			V[x] = V[x] | V[y];
		}

		// 8XY2 - AND Vx, Vy
		// Set Vx = Vx AND Vy
		else if (n == 2)
		{
			V[x] = V[x] & V[y];
		}

		// 8XY3 - XOR Vx, Vy
		// Set Vx = Vx XOR Vy
		else if (n == 3)
		{
			V[x] = V[x] ^ V[y];
		}

		// 8XY4 - ADD Vx, Vy
		// Set Vx = Vx + Vy, set VF = carry bit if overflowed
		else if (n == 4)
		{
			V[0xF] = (int)(V[x]) + (int)(V[y]) > 0xFF;
			V[x] = V[x] + V[y];
		}

		// 8XY5 - SUB Vx, Vy
		// Set Vx =	Vx - Vy, set Vf = NOT borrow (if Vx > Vy)
		else if (n == 5)
		{
			V[0xF] = V[x] > V[y];
			V[x] = V[x] - V[y];
		}

		// 8XY6 - SHR Vx
		// (Ambiguous) Shift Vx right by 1, set Vf = shifted out bit
		else if (n == 6)
		{
			V[0xF] = V[x] & 1; // rightmost bit
			V[x] = V[x] >> 1;
		}

		// 8XY7 - SUBN Vx, Vy
		// Set Vx = Vy - Vx, set Vf = NOT borrow (if Vy > Vx)
		else if (n == 7)
		{
			V[0xF] = V[y] > V[x];
			V[x] = V[y] - V[x];
		}

		// 8XYE - SHL Vx
		// (Ambiguous) Shift Vx left by 1, set Vf = shifted out bit
		else if (n == 0xE)
		{
			V[0xF] = V[x] & 128; // leftmost bit in byte
			V[x] = V[x] << 1;
		}
	}

	// ANNN - LD I, addr
	// Set index register to addr
	else if ((b1 >> 4) == 0xA)
	{
		printf("Set I reg to %03x", addr);
		I = addr;
	}

	// BNNN - JMP V0, addr
	// (Ambiguous, went with old CHIP-8) Program counter is set to addr plus V0
	else if ((b1 >> 4) == 0xB)
	{
		PC = addr + V[0];
	}

	// CXNN - RND Vx, byte
	// Set Vx = random byte AND nn
	else if ((b1 >> 4) == 0xC)
	{
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

	// EX9E - SKP Vx
	// Skip next instruction if key w/ value of Vx is pressed
	else if ((b1 >> 4) == 0xE && b2 == 0x9E)
	{
		if (QueryKey(V[x]))
			PC += 2;
	}

	// EXA1 - SKNP Vx
	// Skip next instruction if key w/ value of Vx is NOT pressed
	else if ((b1 >> 4) == 0xE && b2 == 0xA1)
	{
		if (!QueryKey(V[x]))
			PC += 2;
	}

	// FX07 - LD Vx, DT
	// Set Vx to the current value of the delay timer
	else if ((b1 >> 4) == 0xF && b2 == 0x07)
	{
		V[x] = delayTimer;
	}

	// FX0A - LD Vx, K
	// Blocks instructions until a key is pressed, key stored in Vx
	else if ((b1 >> 4) == 0xF && b2 == 0x0A)
	{
		if (!QueryKey(V[x]))
			PC -= 2;
	}

	// FX15 - LD DT, Vx
	// Set delay timer to value in Vx
	else if ((b1 >> 4) == 0xF && b2 == 0x15)
	{
		delayTimer = V[x];
	}

	// FX18 - LD ST, Vx
	// Set sound timer to value in Vx
	else if ((b1 >> 4) == 0xF && b2 == 0x18)
	{
		soundTimer = V[x];
	}

	// FX1E - ADD I, Vx
	// Set I = I + Vx, set Vf = overflow
	else if ((b1 >> 4) == 0xF && b2 == 0x1E)
	{
		V[0xF] = (int)I + (int)V[x] > 0xFFF;
		I = I + V[x];
	}

	// FX29 - LD F, Vx
	// Set I = location of font sprite for digit Vx
	else if ((b1 >> 4) == 0xF && b2 == 0x29)
	{
		I = 0x50 + V[x] * 6; // font sprite is 6 bytes
	}

	// FX33 - LD B, Vx
	// Stores the 3 digits of the integer representation of Vx @ I, I+1 and I+2
	else if ((b1 >> 4) == 0xF && b2 == 0x33)
	{
		RAM[I] = (V[x] / 100) % 10;
		RAM[I + 1] = (V[x] / 10) % 10;
		RAM[I + 2] = V[x] % 10;
	}

	// FX55 - LD [I], Vx
	// (Ambiguous) Stores values from V0-Vx inclusive @ I-I+x (without changing I)
	else if ((b1 >> 4) == 0xF && b2 == 0x55)
	{
		printf("Store V0-V%x to I\n", V[x]);
		for (int i = 0; i <= x; i++)
			RAM[I + i] = V[i];
	}

	// FX65 - LD Vx, [I]
	// (Ambiguous) Loads values from I-I+x into V0-Vx inclusive (without changing I)
	else if ((b1 >> 4) == 0xF && b2 == 0x65)
	{
		for (int i = 0; i <= x; i++)
			V[i] = RAM[I + i];
	}

	else
	{
		printf("Unknown instruction - %04x", opcode);
	}

	printf("\n");
}