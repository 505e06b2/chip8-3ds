#include "gameloop.h"

static void setInputs(uint32_t keys, int16_t *software_keys, touchPosition pos) { //TODO MERGE WITH TOUCH INPUT - TOUCH HAVE PRIORITY FOR BLOCKING
	int key_index = -1;
	static int last_pressed_key = -1; //acts like global const, but scoped

	#define CHECKKEY(hardware, software) ((keys & hardware) && (software_keys[software] >= 0))
	#define SETSWKEY(key) {key_index = software_keys[key]; chip8_keys_down[key_index] = 1;}

	if(CHECKKEY(KEY_UP,    SOFTWARE_UP))     SETSWKEY(SOFTWARE_UP)
	if(CHECKKEY(KEY_DOWN,  SOFTWARE_DOWN))   SETSWKEY(SOFTWARE_DOWN)
	if(CHECKKEY(KEY_LEFT,  SOFTWARE_LEFT))   SETSWKEY(SOFTWARE_LEFT)
	if(CHECKKEY(KEY_RIGHT, SOFTWARE_RIGHT))  SETSWKEY(SOFTWARE_RIGHT)

	if(CHECKKEY(KEY_A,     SOFTWARE_A))      SETSWKEY(SOFTWARE_A)
	if(CHECKKEY(KEY_B,     SOFTWARE_B))      SETSWKEY(SOFTWARE_B)
	if(CHECKKEY(KEY_X,     SOFTWARE_X))      SETSWKEY(SOFTWARE_X)
	if(CHECKKEY(KEY_Y,     SOFTWARE_Y))      SETSWKEY(SOFTWARE_Y)

	for(int i = 0; i < sizeof(KEYPAD_KEYS)/sizeof(KEYPAD_KEYS[0]); i++) {
		uint16_t keypad_x = KEYPAD_KEYS[i].px - KEYPAD_KEY_OFFSET_X; //KEYPAD_KEYS is in graphics.h/c
		uint16_t keypad_y = KEYPAD_KEYS[i].py - KEYPAD_KEY_OFFSET_Y;
		if(pos.px > keypad_x && pos.px < keypad_x + KEYPAD_KEY_WIDTH &&
			pos.py > keypad_y && pos.py < keypad_y + KEYPAD_KEY_HEIGHT) {
			key_index = i;
			chip8_keys_down[key_index] = 1;
			break;
		}
	}

	if(key_index == -1) {
		last_pressed_key = -1;
		return;
	}

	if(chip8_blocking && last_pressed_key != key_index) {
		*chip8_blocking = key_index;
		chip8_blocking = NULL;
	}
	last_pressed_key = key_index;
}

void gameLoop(RomFile *rom_info) {
	if(chip8_load(rom_info->rom_path) != NULL) return;
	graphics_chip8_render();

	uint32_t keys_down = 0;
	uint64_t timer_ms = 0;
	uint64_t cycle_ms = 0;
	touchPosition touch_pos;

	while(aptMainLoop()) {
		hidScanInput();

		memset(chip8_keys_down, 0, sizeof(chip8_keys_down)); //reset to 0 for now

		keys_down = hidKeysDown() | hidKeysHeld(); //merge both
		hidTouchRead(&touch_pos);

		if(keys_down & KEY_START) break;
		setInputs(keys_down, rom_info->software_keys, touch_pos);

		if(!chip8_blocking) {
			uint64_t timestamp = osGetTime();

			if(timestamp - timer_ms >= TIMER_DELAY_MS) {
				chip8_decreaseTimers();
				timer_ms = timestamp;
			}

			if(timestamp - cycle_ms >= CYCLE_DELAY_MS) {
				if(chip8_cycle()) return;
				cycle_ms = timestamp;
			}
		}

        if(chip8_drawFlag()) graphics_chip8_render(); // blocks with vsync
	}
}
