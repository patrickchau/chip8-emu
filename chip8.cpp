#include "chip8.h"
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <conio.h>
#include <Windows.h>

chip8::chip8() {

}

chip8::~chip8() {

}

bool chip8::initialize() {
	// initialize registers and memory
	pc = 0x200;	// initialize to address 512.
	opcode = 0;	// set all variables to 0 at the beginning
	I = 0;		// Index pointer : used for choosing sprites
	sp = 0;		// stack pointer

	// clear stack
	for (int k = 0; k < sizeof(stack)/sizeof(stack[0]); k++) {
		stack[k] = 0;
	}

	// clear display
	for (int k = 0; k < sizeof(gfx) / sizeof(gfx[0]); k++) {
		gfx[k] = 0;
	}

	// clear keys 
	for (int k = 0; k < (sizeof(key) / sizeof(key[0])); k++) {
		key[k] = 0;
	}

	for (int k = 0; k < (sizeof(reg) / sizeof(reg[0])); k++) {
		reg[k] = 0;
	}

	// clear memory
	for (int k = 0; k < sizeof(memory) / sizeof(memory[0]); k++) {
		memory[k] = 0;
	}

	// load fontset
	for (int k = 0; k < 80; k++) {
		memory[k] = chip8_fontset[k];
	}

	// reset timers
	delay_timer = 0;
	sound_timer = 0;
	// start by drawing the empty display
	drawFlag = true;

	return true;
}

// given the current pc, fetch the opcode and execute
void chip8::emulateCycle() {
	// fetch opcode
	opcode = memory[pc] << 8 | memory[pc + 1];
	//printf("Current opcode: %04x , Current PC: %d \n", opcode, pc);
	unsigned short vx;
	unsigned short vy;
	unsigned short x;
	unsigned short y;
	// decode and execute opcode: using a lookup table
	switch ( opcode & 0xF000 ) {
	case 0x0000:
		switch (opcode & 0x00FF) {
		case 0x00E0:	// clears the screen OKAY
			for (int j = 0; j < sizeof(gfx) / sizeof(gfx[0]); j++) {
				gfx[j] = 0;
			}
			drawFlag = true;
			pc += 2;
			break;
		case 0x00EE:	// returns from the subroutine	OKAY
			// then pops the stack
			sp -= 1;
			// sets pc to the address at the bottom of the stack
			pc = stack[sp];
			pc += 2;
			break;
		default:	
			printf("Unknown opcode [0x0000]: 0x%X\n", opcode);
			break;
		}
		break;
	case 0x1000:	// jumps to address NNN OKAY
		pc = (opcode & 0x0FFF);
		break;
	case 0x2000:	// call the subroutine at address NNN OKAY
		// push the current pc onto the stack and update stack pointer
		stack[sp] = pc;
		sp += 1;
		// then jump to address NNN
		pc = (opcode & 0x0FFF);
		break;
	case 0x3000:	// skip the next instruction if VX = NN
		vx = reg[(opcode & 0x0F00) >> 8];
		if (vx == (opcode & 0x00FF)) {
			pc += 4;
		}
		else {
			pc += 2;
		}
		break;
	case 0x4000:
		x = (opcode & 0x0F00) >> 8;
		vx = reg[x];
		if (vx != (opcode & 0x00FF)) {
			pc += 4;
		}
		else {
			pc += 2;
		}
		break;
	case 0x5000:
		x = (opcode & 0x0F00) >> 8;
		y = (opcode & 0x00F0) >> 4;
		vx = reg[x];
		vy = reg[y];
		if (vx == vy) {
			pc += 4;
		}
		else {
			pc += 2;
		}
		break;
	case 0x6000:
		x = (opcode & 0x0F00) >> 8;
		reg[x] = (opcode & 0x00FF);
		pc += 2;
		break;
	case 0x7000:
		x = (opcode & 0x0F00) >> 8;
		reg[x] += (opcode & 0x00FF);
		pc += 2;
		break;
	case 0x8000:
		switch (opcode & 0x000F) {
		case 0x0000: // 0x8XY0: sets VX to VY
			x = (opcode & 0x0F00) >> 8;
			y = (opcode & 0x00F0) >> 4;
			reg[x] = reg[y];
			pc += 2;
			break;
		case 0x0001: // vx | vy
			x = (opcode & 0x0F00) >> 8;
			y = (opcode & 0x00F0) >> 4;
			reg[x] = reg[x] | reg[y];
			pc += 2;
			break;
		case 0x0002:	// vx & vy
			x = (opcode & 0x0F00) >> 8;
			y = (opcode & 0x00F0) >> 4;
			reg[x] = reg[x] & reg[y];
			pc += 2;
			break;
		case 0x0003:	// vx ^= vy
			x = (opcode & 0x0F00) >> 8;
			y = (opcode & 0x00F0) >> 4;
			reg[x] = reg[x] ^ reg[y];
			pc += 2;
			break;
		case 0x0004:	//vx += vy
			x = (opcode & 0x0F00) >> 8;
			y = (opcode & 0x00F0) >> 4;
			reg[x] += reg[y];
			pc += 2;
			break;
		case 0x0005:	// vx -= vy
			x = (opcode & 0x0F00) >> 8;
			y = (opcode & 0x00F0) >> 4;
			if (reg[y] > reg[x]) {
				reg[15] = 0;
			}
			else {
				reg[15] = 1;
			}
			reg[x] -= reg[y];
			pc += 2;
			break;
		case 0x0006:	// stores LSB of VX in VF and shifts VX to right by 1
			x = (opcode & 0x0F00) >> 8;
			reg[x] >>= 1;
			pc += 2;
			break;
		case 0x0007:	// sets vx = vy - vx. if there is a borrow, set reg15 to 0
			x = (opcode & 0x0F00) >> 8;
			y = (opcode & 0x00F0) >> 4;
			if (reg[x] > reg[y]) {
				reg[15] = 0;
			}
			else {
				reg[15] = 1;
			}
			reg[x] = reg[y] - reg[x];
			pc += 2;
			break;
		case 0x000E:	// stores the most significant bit of VX in VF and then shifts VX to the left by 1
			x = (opcode & 0x0F00) >> 8;
			reg[15] = reg[x] >> 7;
			reg[x] <<= 1;
			pc += 2;
			break;
		}
		break;
	case 0x9000:	// skips the next instruction if vx != vy
		x = (opcode & 0x0F00) >> 8;
		y = (opcode & 0x00F0) >> 4;
		vx = reg[x];
		vy = reg[y];
		if (vx != vy) {
			pc += 4;
		}
		else {
			pc += 2;
		}
		break;
	case 0xA000: // ANNN: sets I to the address NNN
		I = opcode & 0x0FFF;
		pc += 2;
		break;
	case 0xB000: // BNNN: jumps to the address NNN + V0
		pc = reg[0] + (opcode & 0x0FFF);
		break;
	case 0xC000: // sets VX to result of bitwise and op on a random number
		x = (opcode & 0x0F00) >> 8;
		reg[x] = ( rand() & 0x00FF ) & (opcode & 0x00FF);
		pc += 2;
		break;
	case 0xD000: // DXYN: draws a coordinate at (VX, VY) with width of 8 pixels, height of N pixels.
		// note VF is set to 1 if any screen pixels are flipped from set to unset when sprite is drawn, 0 if not
		// state of a pixel is set by XOR
		unsigned short pos_x;
		pos_x = reg[(opcode & 0x0F00) >> 8];
		unsigned short pos_y;
		pos_y = reg[(opcode & 0x00F0) >> 4];
		unsigned short height;
		height = opcode & 0x000F;
		unsigned short sprite;
		reg[15] = 0;
		// sprite will be 8 pixels wide, so we choose the y position and x position and draw 8 out from there
		// height determines how many y lines we need to draw
		// 'I' will be pointing to each 8 wide sprite that we need to draw, stored in memory
		for (int y_off = 0; y_off < height; y_off++) {
			sprite = memory[I + y_off];
			for (int x_off = 0; x_off < 8; x_off++) {
				if ((sprite & (0x80 >> x_off)) != 0) {	// checking each bit in sprite and see if it is 1. if so, draw
					// if the position we are drawing to is already drawn, collision flag set
					if (gfx[ (pos_x + x_off) + ((pos_y + y_off)*64) ] == 1) {
						reg[15] = 1;
					}
					// then we xor that position
					gfx[pos_x + x_off + ((pos_y + y_off) * 64)] ^= 1;
				}
			}
		}
		drawFlag = true;
		pc += 2;
		break;
	case 0xE000:
		switch (opcode & 0x00FF) {
		case 0x009E:	// skips the next instruction if the key stored in vx is pressed
			x = (opcode & 0x0F00) >> 8;
			vx = reg[x];
			if (key[vx] == 1) {
				pc += 4;
			}
			else {
				pc += 2;
			}
			break;
		case 0x00A1:	// skips the next instruction if the key stored in vx isnt pressed
			x = (opcode & 0x0F00) >> 8;
			vx = reg[x];
			if (key[vx] == 0) {
				pc += 4;
			}
			else {
				pc += 2;
			}
			break;
		default:
			printf("Unknown opcode [0xE000]: 0x%X\n", opcode);
		}
		break;
	case 0xF000:
		switch (opcode & 0x00FF) {
		case 0x0007:	// sets VX to value of the delay timer
			x = (opcode & 0x0F00) >> 8;
			reg[x] = delay_timer;
			pc += 2;
			break;
		case 0x000A:	// detects a key press
			x = (opcode & 0x0F00) >> 8;
			vx = reg[x];
			bool keyPressed;
			keyPressed = false;
			// a key is pressed if any of the keys are set to 1.
			for (int k = 0; k < (sizeof(key) / sizeof(key[0])); k++) {
				if (key[k] != 0) {
					reg[x] = k;
					keyPressed = true;
				}
			}
			if (!keyPressed) {
				return;
			}
			pc += 2;
			break;
		case 0x0015:	//	sets the delay timer to VX
			x = (opcode & 0x0F00) >> 8;
			delay_timer = reg[x];
			pc += 2;
			break;
		case 0x0018:	//sets the sound timer to VX
			x = (opcode & 0x0F00) >> 8;
			sound_timer = reg[x];
			pc += 2;
			break;
		case 0x001E:	// adds VX to I, VF is set to 1 if there is an integer overflow
			x = (opcode & 0x0F00) >> 8;
			vx = reg[x];
			if ( I + vx > 0xFFF ) {	// then there was an overflow
				reg[15] = 1;
			}
			else {
				reg[15] = 0;
			}
			I += vx;
			pc += 2;
			break;
		case 0x0029:	// sets	I to the location of the sprite for the character in VX
			x = (opcode & 0x0F00) >> 8;
			I = reg[x] * 5;	// because each sprite is 5 wide
			pc += 2;
			break;
		case 0x0033:	// stores the binary coded decimal of VX
			x = (opcode & 0x0F00) >> 8;
			vx = reg[x];
			memory[I] = (vx / 100);
			memory[I + 1] = (vx / 10) % 10;
			memory[I + 2] = (vx % 100) % 10;
			pc += 2;
			break;
		case 0x0055:	// dumps registers V0 to VX to memory address of I
			x = (opcode & 0x0F00) >> 8;
			for (int k = 0; k < (x + 1); k++) {
				memory[I + k] = reg[k];
			}
			I = I + (x + 1);
			pc += 2;
			break;
		case 0x0065:	// fills V0 to VX with values from memory starting at address I.
			x = (opcode & 0x0F00) >> 8;
			for (int k = 0; k < (x + 1); k++) {
				reg[k] = memory[I + k];
			}
			I = I + (x + 1);
			pc += 2;
			break;
		}
		break;
	default:
		printf("Unknown opcode: 0x%X\n", opcode);
	}

	// update timers
	if (delay_timer > 0) {
		--delay_timer;
	}
	if (sound_timer > 0) {
		if (sound_timer == 1) {
			printf("beep\n");
		}
		--sound_timer;
	}
}

bool chip8::loadGame() {
	char* buffer = NULL;
	FILE* file = fopen("invaders.c8", "rb");
	long size;
	size_t result;
	if (file == NULL) {
		// if null, file doesn't exist and we exit
		fputs("File error, does not exist", stderr);
		exit(1);
	}
	// otherwise opening the file worked
	// now we load the file into memory.
	fseek(file, 0, SEEK_END);
	size = ftell(file);	// gets the number of bytes from the start of the file.
	rewind(file);		// return to the start of the file

	//based on the found size of the file, allocate enough memory to store the whole file
	buffer = (char*)malloc(sizeof(char) * size);

	// make sure that it's not null
	if (buffer == NULL) {
		fputs("Memory error", stderr);
		exit(2);
	}

	result = fread(buffer, 1, size, file);

	if (result != size) {
		fputs("Reading error", stderr);
		exit(3);
	}

	// we should make sure that the rom that we read is not too large for memory
	if (size < 4096 - 0x200) {
		for (int i = 0; i < size; i++) {
			memory[i + 512] = buffer[i];
		}
	}
	else {
		printf("ROM too large for memory.");
	}

	free(buffer);
	fclose(file);
	return 1;
}
