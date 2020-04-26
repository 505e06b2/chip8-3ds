#ifndef CHIP8_H
#define CHIP8_H

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define PC_START 0x200
#define FONTSET_START 0x050

#define CHIP_WIDTH 64
#define CHIP_HEIGHT 32

#define DISPLAY_BITMASK_TYPE_BITS (sizeof(uint32_t)*8)

uint32_t chip8_display[(CHIP_WIDTH * CHIP_HEIGHT) / DISPLAY_BITMASK_TYPE_BITS];
uint8_t *chip8_blocking;
uint8_t chip8_keys_down[16];

#ifdef DEBUG
	#define D_LOG(str, ...) printf((str), ##__VA_ARGS__)
#else
	#define D_LOG(str, ...)
#endif

typedef struct chip8_Internals {
	uint16_t opcode;
	uint8_t memory[4096];
	uint8_t V[16]; //registers

	uint16_t I; //index register
	uint16_t program_counter;

	uint8_t delay_timer; //60hz
	uint8_t sound_timer;

	uint16_t stack[16];
	uint16_t stack_pointer;
} chip8_Internals;

const char *chip8_load(const char *);
uint8_t chip8_cycle();
void chip8_decreaseTimers();

uint8_t chip8_display_draw(uint8_t, uint8_t, uint8_t);
uint8_t chip8_drawFlag();
void chip8_display_clear();

//debugging

void chip8_memoryDump();
void chip8_printInternals();

#endif
