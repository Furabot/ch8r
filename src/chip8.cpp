#include "chip8.h"
#include <cstdint>		//uint types
#include <stdio.h>      //NULL
#include <stdlib.h>     //srand
#include <time.h>		//time function

uint8_t fontset[80] = {

	0xF0, 0x90, 0x90, 0x90, 0xF0, //0
	0x20, 0x60, 0x20, 0x20, 0x70, //1
	0xF0, 0x10, 0xF0, 0x80, 0xF0, //2
	0xF0, 0x10, 0xF0, 0x10, 0xF0, //3
	0x90, 0x90, 0xF0, 0x10, 0x10, //4
	0xF0, 0x80, 0xF0, 0x10, 0xF0, //5
	0xF0, 0x80, 0xF0, 0x90, 0xF0, //6
	0xF0, 0x10, 0x20, 0x40, 0x40, //7
	0xF0, 0x90, 0xF0, 0x90, 0xF0, //8
	0xF0, 0x90, 0xF0, 0x10, 0xF0, //9
	0xF0, 0x90, 0xF0, 0x90, 0x90, //A
	0xE0, 0x90, 0xE0, 0x90, 0xE0, //B
	0xF0, 0x80, 0x80, 0x80, 0xF0, //C
	0xE0, 0x90, 0x90, 0x90, 0xE0, //D
	0xF0, 0x80, 0xF0, 0x80, 0xF0, //E
	0xF0, 0x80, 0xF0, 0x80, 0x80  //F

};

void CHIP8::reset() {

	pc = 0x200; //program start address
	opcode = 0; //reset opcode
	I = 0; //clear index
	sp = 0; //clear stack pointer

	delayTimer = 0; //reset timers
	soundTimer = 0;

	for (int i = 0; i < 2048; ++i) //clear display
		gfx[i] = 0;

	for (int i = 0; i < 4096; ++i) //clear memory
		memory[i] = 0;

	for (int i = 0; i < 16; ++i) { //clear stack, V register, and keypress
		stack[i] = 0;
		V[i] = 0;
		key[i] = 0;
	}

	for (int i = 0; i < 80; ++i) //load fonts in start of memory
		memory[i] = fontset[i];

	drawFlag = true; //clear display

	srand(time(NULL));
	
}

bool CHIP8::loadROM(const char * file) {

	reset();
	printf("Loading: %s\n", file);
		
	// Open file
	FILE * pFile = fopen(file, "rb");
	if (pFile == NULL)
	{
		fputs ("File error", stderr); 
		return false;
	}

	// Check file size
	fseek(pFile , 0 , SEEK_END);
	long lSize = ftell(pFile);
	rewind(pFile);
	printf("Filesize: %d\n", (int)lSize);
	
	// Allocate memory to contain the whole file
	char * buffer = (char*)malloc(sizeof(char) * lSize);
	if (buffer == NULL) 
	{
		fputs ("Memory error", stderr); 
		return false;
	}

	// Copy the file into the buffer
	size_t result = fread (buffer, 1, lSize, pFile);
	if (result != lSize) 
	{
		fputs("Reading error",stderr); 
		return false;
	}

	// Copy buffer to Chip8 memory
	if((4096-512) > lSize)
	{
		for(int i = 0; i < lSize; ++i)
			memory[i + 512] = buffer[i];
	}
	else
		printf("Error: ROM too big for memory");
	
	// Close file, free buffer
	fclose(pFile);
	free(buffer);

	return true;
}

void CHIP8::emulateCycle() {

	//Fetch
	opcode = memory[pc] << 8 | memory[pc + 1];

	int x = (opcode & 0x0F00) >> 8;
	int y = (opcode & 0x00F0) >> 4;

	//Decode and Execute
	switch (opcode & 0xF000){

	case 0x0000:
		switch (opcode & 0x000F) {

		case 0x0000: //0x00E0 (CLS): Clear Display
			for (int i = 0; i < 2048; ++i)
				gfx[i] = 0;
			drawFlag = true;
			pc += 2;
			break;

		case 0x000E: //0x00EE (RET): Return from Subroutine
			--sp;
			pc = stack[sp];
			pc += 2;
			break;

		default:
			printf("Unknown opcode: 0x%X\n", opcode);
		}
		break;

	case 0x1000: //0x1NNN (JP): Jump to address NNN
		pc = opcode & 0x0FFF;
		break;

	case 0x2000: //0x2NNN (CALL): Call subroutine at NNN
		stack[sp] = pc;
		++sp;
		pc = opcode & 0x0FFF;
		break;

	case 0x3000: //0x3XKK (SE): Skip next instruction if Vx = KK
		if(V[x] == (opcode & 0x00FF))
			pc += 4;
		else
			pc += 2;
		break;

	case 0x4000: //0x4XKK (SNE): Skip next instruction if Vx != KK
		if (V[x] != (opcode & 0x00FF))
			pc += 4;
		else
			pc += 2;
		break;

	case 0x5000: //0x5XY0 (SE): Skip next instruction if Vx = Vy
		if (V[x] == V[y])
			pc += 4;
		else
			pc += 2;
		break;

	case 0x6000: //0x6XKK (LD): Load value KK into Vx
		V[x] = opcode & 0x00FF;
		pc += 2;
		break;

	case 0x7000: //0x7xKK (ADD): Set Vx = Vx + kk
		V[x] += opcode & 0x00FF;
		pc += 2;
		break;

	case 0x8000:
		switch (opcode & 0x000F) {

		case 0x0000: //0x8XY0 (LD): Load Vy's value into Vx
			V[x] = V[y];
			pc += 2;
			break;

		case 0x0001: //0x8XY1 (OR): Set Vx to (Vx OR Vy) 
			V[x] |= V[y];
			pc += 2;
			break;

		case 0x0002: //0x8XY2 (AND): Set Vx to (Vx AND Vy) 
			V[x] &= V[y];
			pc += 2;
			break;

		case 0x0003: //0x8XY3 (XOR): Set Vx to (Vx XOR Vy) 
			V[x] ^= V[y];
			pc += 2;
			break;

		case 0x0004: //0x8XY4 (ADD): Set Vx to (Vx + Vy); VF is set when there's a carry
			if (V[y] > 0xFF - V[x])
				V[0xF] = 1;
			else
				V[0xF] = 0;
			V[x] += V[y];
			pc += 2;
			break;
			

		case 0x0005: //0x8XY5 (SUB): Set Vx to (Vx - Vy); VF is cleared when there's a borrow
			if(V[y] > V[x])
				V[0xF] = 0;
			else
				V[0xF] = 1;
			V[x] -= V[y];
			pc += 2;
			break;

		case 0x0006: //0x8XY6 (SHR): Store LSB of Vx in VF, then right shift Vx by 1
			V[0xF] = V[x] & 0x1;
			V[x] >>= 1;
			pc += 2;
			break;

		case 0x0007: //0x8XY7 (SUBN): Set Vx to (Vy - Vx); VF is cleared when there's a borrow
			if (V[x] > V[y])
				V[0xF] = 0;
			else
				V[0xF] = 1;
			V[x] = V[y] - V[x];
			pc += 2;
			break;

		case 0x000E: //0x8XYE (SHL): Store MSB of Vx in VF, then left shift Vx by 1
			V[0xF] = V[x] >> 7;
			V[x] <<= 1;
			pc += 2;
			break;

		default:
			printf("Unknown opcode: 0x%X\n", opcode);
		}
		break;

	case 0x9000: //0x9XY0 (SNE): Skip next instruction if Vx != Vy
		if (V[x] != V[y])
			pc += 4;
		else
			pc += 2;
		break;

	case 0xA000: //0xANNN (LD): Load I with NNN
		I = opcode & 0x0FFF;
		pc += 2;
		break;

	case 0xB000: //0xBNNN (JP): Jump to NNN + V0
		pc = (opcode & 0x0FFF) + V[0];
		break;

	case 0xC000: //CXKK (RND): Generate a random number between 0 and 255, AND with KK and store in Vx
		V[x] = (rand() % 0xFF) & (opcode & 0x00FF);
		pc += 2;
		break;

	case 0xD000: //DXYN (DRAW): Draw sprite at coordinates Vx, Vy, with height N; Set VF to 1 if 1 changes to 0
	{
		uint16_t x_coord = V[x]; //x coordinate location
		uint16_t y_coord = V[y]; //y coordinate location
		uint16_t n = opcode & 0x000F; //height 
		uint16_t pixel;

		V[0xF] = 0;
		for (int yline = 0; yline < n; ++yline) {
			pixel = memory[I + yline]; //one row of sprite in memory
			for (int xline = 0; xline < 8; ++xline) {
				if ((pixel & (0x80 >> xline)) != 0) { //check if a pixel in sprite is 1
					if (gfx[(x_coord + xline + ((y_coord + yline) * 64))] == 1) //check if current pixel displayed is 1
						V[0xF] = 1; //collision
					gfx[(x_coord + xline + ((y_coord + yline) * 64))] ^= 1; //if currently displayed pixel is 0, it'll be changes to 1. If already 1, then flip to 0, i.e. XOR with 1
				}
			}
		}

		drawFlag = true;
		pc += 2;
	}
	break;

	case 0xE000: 
		switch (opcode & 0x00FF) {

		case 0x009E: //EX9E (SKP): Skip next instruction if key with value Vx is pressed
			if (key[V[x]] != 0)
				pc += 4;
			else
				pc += 2;
			break;

		case 0x00A1: //EXA1 (SKNP): Skip next instruction if key with value Vx is not pressed
			if (key[V[x]] == 0)
				pc += 4;
			else
				pc += 2;
			break;

		default:
			printf("Unknown opcode: 0x%X\n", opcode);
		}
		break;

	case 0xF000:
		switch (opcode & 0x00FF){

		case 0x0007: //FX07 (LD): Load value of delay timer into Vx
			V[x] = delayTimer;
			pc += 2;
			break;

		case 0x000A: //FX0A (LD): Wait for a keypress, then store value in Vx
		{
			bool keyPressed = false;

			for (int i = 0; i < 16; ++i) {
				if (key[i] != 0)
				{
					V[x] = i;
					keyPressed = true;
				}
			}

			if (!keyPressed)
				return;
			pc += 2;
		}
		break;

		case 0x0015: //FX15 (LD): Load delay timer with Vx value
			delayTimer = V[x];
			pc += 2;
			break;

		case 0x0018: //FX18 (LD): Load sound timer with Vx value
			soundTimer = V[x];
			pc += 2;
			break;

		case 0x001E: //FX1E (ADD): Add Vx value to I and store in I. VF is set when range overflows
			if (I + V[x] > 0xFFF)
				V[0xF] = 1;
			else
				V[0xF] = 0;

			I += V[x];
			pc += 2;
			break;

		case 0x0029: //FX29 (LD): Load the address of font sprite in memory from Vx in I. Vx hold the digit of font (0-F), so multiply by 5 to get address (font sprites are 5 bytes).
			I = V[x] * 0x5;
			pc += 2;
			break;

		case 0x0033: //FX33 (LD): Load the BCD representation of Vx in memory. Converts to decimal and stores each place in I, I+1, and I+2 memory location
			memory[I] = V[x] / 100;
			memory[I + 1] = (V[x] / 10) % 10;
			memory[I + 2] = (V[x] % 100) % 10;
			pc += 2;
			break;

		case 0x0055: //FX55 (LD): Store values in registers V0-Vx starting at location I. (Optional for compatibility?) Increment I for every address loaded.
			for (int i = 0; i <= x; ++i) {
				memory[I] = V[i];
				++I; //Apparently for compatibility with older games
			}
			pc += 2;
			break;

		case 0x0065: //FX55 (LD): Load values in registers V0-Vx starting at location I. (Optional for compatibility?) Increment I for every address loaded.
			for (int i = 0; i <= x; ++i) {
				V[i] = memory[I];
				++I;
			}
			pc += 2;
			break;

		default:
			printf("Unknown opcode: 0x%X\n", opcode);
		}
		break;

	default:
		printf("Unknown opcode: 0x%X\n", opcode);

	}

	//Update Timers
	if (delayTimer > 0)
		--delayTimer;

	if (soundTimer > 0) {
		//add sound here
		--soundTimer;
	}
}

void CHIP8::CLIRender() {

	for (int y = 0; y < 32; ++y) {

		for (int x = 0; x < 64; ++x) {

			if (gfx[(y * 64) + x] == 0)
				printf("0");
			else
				printf(" ");
		}
		printf("\n");
	}
	printf("\n");
}
