#ifndef GAMELOOP_H
#define GAMELOOP_H

#include "chip8.h"
#include "graphics.h"

#define TIMER_DELAY_MS (1000 / 60)
#define CYCLE_DELAY_MS (1000 / 500)

#define SOFTWARE_UP 0
#define SOFTWARE_DOWN 1
#define SOFTWARE_LEFT 2
#define SOFTWARE_RIGHT 3
#define SOFTWARE_A 4
#define SOFTWARE_B 5
#define SOFTWARE_X 6
#define SOFTWARE_Y 7

typedef struct RomFile {
	struct RomFile *prev;
	struct RomFile *next;

	C2D_TextBuf name_buffer;
	C2D_Text name_text;
	char *name_str;

	C2D_TextBuf desc_buffer;
	C2D_Text desc_text;
	char *desc_str;

	char *rom_path;
	int16_t software_keys[8]; //up,down,left,right,a,b,x,y
} RomFile;

void gameLoop(RomFile *);

#endif
