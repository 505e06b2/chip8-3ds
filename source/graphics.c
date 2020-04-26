#include "graphics.h"

#define PIXEL_WIDTH TOP_SCREEN_WIDTH/CHIP_WIDTH
#define PIXEL_HEIGHT TOP_SCREEN_HEIGHT/CHIP_HEIGHT

static C2D_SpriteSheet game_bottom_t3x;
static C2D_Sprite game_bottom_bg;

const touchPosition KEYPAD_KEYS[16] = {
	{117,175}, //0
	{44,  10}, //1
	{114, 10}, //2
	{185, 10}, //3
	{44,  65}, //4
	{114, 65}, //5
	{185, 65}, //6
	{44, 120}, //7
	{114,120}, //8
	{185,120}, //9

	{44, 175}, //a
	{185,175}, //b
	{255, 10}, //C
	{255, 65}, //d
	{255,120}, //e
	{255,175}, //f
};

void graphics_init() {
	off = C2D_Color32(0x00, 0x00, 0x00, 0xFF);
	on = C2D_Color32(0xFF, 0xFF, 0xFF, 0xFF);

	gfxInitDefault();
	C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
	C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
	C2D_Prepare();

	game_bottom_t3x = C2D_SpriteSheetLoad("romfs:/gfx/game_bottom.t3x"); //romfs init in main
	C2D_SpriteFromSheet(&game_bottom_bg, game_bottom_t3x, 0);

	top_screen = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
	bottom_screen = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);
}

void graphics_exit() {
	C2D_SpriteSheetFree(game_bottom_t3x);
	C2D_Fini();
	C3D_Fini();
	gfxExit();
}

void graphics_chip8_render() {
	C3D_FrameBegin(C3D_FRAME_SYNCDRAW);

	C2D_TargetClear(top_screen, off);
	C2D_SceneBegin(top_screen);
		int display_index = 0;
		for(int y = 0; y < CHIP_HEIGHT; y++) {
			for(int x = 0; x < CHIP_WIDTH; x++) {
				display_index = (x + (y * CHIP_WIDTH)) / DISPLAY_BITMASK_TYPE_BITS;
				int pos = x % DISPLAY_BITMASK_TYPE_BITS;
				if(chip8_display[display_index] & (1 << pos)) {
					C2D_DrawRectSolid(
						x*PIXEL_WIDTH + PIXEL_PADDING, y*PIXEL_HEIGHT + PIXEL_PADDING,
						0,
						PIXEL_WIDTH - PIXEL_PADDING, PIXEL_HEIGHT - PIXEL_PADDING,
						on);
				}
			}
		}

	C2D_TargetClear(bottom_screen, off);
	C2D_SceneBegin(bottom_screen);
		for(int i = 0; i < sizeof(chip8_keys_down); i++) {
			if(chip8_keys_down[i]) {
				uint16_t keypad_x = KEYPAD_KEYS[i].px - KEYPAD_KEY_OFFSET_X;
				uint16_t keypad_y = KEYPAD_KEYS[i].py - KEYPAD_KEY_OFFSET_Y;
				C2D_DrawRectSolid(keypad_x, keypad_y, 0,  KEYPAD_KEY_WIDTH, KEYPAD_KEY_HEIGHT, on);
			}
		}
		C2D_DrawSprite(&game_bottom_bg);
	C3D_FrameEnd(0);
}
