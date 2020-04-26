#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <citro2d.h>
#include <stdint.h>

#include "chip8.h"

#define PIXEL_PADDING 1

#define TOP_SCREEN_WIDTH  400
#define TOP_SCREEN_HEIGHT 240

#define BOTTOM_SCREEN_WIDTH 320
#define BOTTOM_SCREEN_HEIGHT 240

uint32_t off;
uint32_t on;

C3D_RenderTarget *top_screen;
C3D_RenderTarget *bottom_screen;

#define KEYPAD_KEY_WIDTH 60
#define KEYPAD_KEY_HEIGHT 50
#define KEYPAD_KEY_OFFSET_X 20
#define KEYPAD_KEY_OFFSET_Y 8

const touchPosition KEYPAD_KEYS[16];

void graphics_init();
void graphics_exit();
void graphics_chip8_render();

#endif
