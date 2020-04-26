#include <time.h>
#include <dirent.h>

#include "graphics.h"
#include "gameloop.h"
#include "ini.h"

#define BASE_PATH "sdmc:/chip8/"
#define ROM_EXT "ch8"

#define TEXT_BUFFER_SIZE 256

static int handler(void* v_ptr, const char* section, const char* name, const char* value) {
	RomFile *rom_file = (RomFile *)v_ptr;

    #define MATCH(s, n) (strcmp(section, s) == 0 && strcmp(name, n) == 0)
    #define SETSTR(s, v) if(s) {s = realloc(s, strlen(s)+strlen(v)+2);} else {s = calloc(1, strlen(v)+2);} strcat(s, v); strcat(s, "\n"); //+2 for \n
    #define SETSWKEY(k, v) rom_file->software_keys[k] = strtol(v, NULL, 16);

	if(MATCH("details", "name")) {
		SETSTR(rom_file->name_str, value);
	} else if(MATCH("details", "description")) {
		SETSTR(rom_file->desc_str, value);

	} else if(MATCH("keys", "up")) {
		SETSWKEY(SOFTWARE_UP, value);
	} else if(MATCH("keys", "down")) {
		SETSWKEY(SOFTWARE_DOWN, value);
	} else if(MATCH("keys", "left")) {
		SETSWKEY(SOFTWARE_LEFT, value);
	} else if(MATCH("keys", "right")) {
		SETSWKEY(SOFTWARE_RIGHT, value);

	} else if(MATCH("keys", "a")) {
		SETSWKEY(SOFTWARE_A, value);
	} else if(MATCH("keys", "b")) {
		SETSWKEY(SOFTWARE_B, value);
	} else if(MATCH("keys", "x")) {
		SETSWKEY(SOFTWARE_X, value);
	} else if(MATCH("keys", "y")) {
		SETSWKEY(SOFTWARE_Y, value);

    } else {
        return 0;  /* unknown section/name, error */
    }

    return 1;
}

RomFile *getRomFiles() {
	char string_buffer[4096];
	struct dirent **namelist;
	RomFile *start = NULL;
	RomFile *prev = NULL;

	int n = scandir(BASE_PATH, &namelist, 0, alphasort);
	for(int i = 0; i < n; i++) {
		strcpy(string_buffer, BASE_PATH); //kept simple
		strcat(string_buffer, namelist[i]->d_name);
		free(namelist[i]);

		char *display_string = string_buffer + sizeof(BASE_PATH)-1;
		char *last_dot = strrchr(display_string, '.');

		//check if an actual rom file
		if(!last_dot || strcmp(last_dot+1, ROM_EXT) != 0) continue;

		//create RomFile - linked list stuff
		RomFile *current = calloc(1, sizeof(RomFile));
		if(!start) start = current; //used once
		current->prev = prev;
		current->next = NULL;
		if(current->prev) current->prev->next = current;
		prev = current;

		//do actual useful stuff
		for(int i = 0; i < sizeof(current->software_keys)/sizeof(current->software_keys[0]); i++) current->software_keys[i] = -1; //initialize to -1

		current->rom_path = malloc(strlen(string_buffer)+1);
		strcpy(current->rom_path, string_buffer);

		strcpy(last_dot+1, "ini");
		ini_parse(string_buffer, handler, current); //if < 0 something failed, but we want defaults anyways

		//defaults
		*last_dot = '\0';

		current->name_buffer = C2D_TextBufNew(TEXT_BUFFER_SIZE);
		C2D_TextParse(&(current->name_text), current->name_buffer, (current->name_str) ? current->name_str : display_string);
		C2D_TextOptimize(&(current->name_text));

		current->desc_buffer = C2D_TextBufNew(TEXT_BUFFER_SIZE);
		C2D_TextParse(&(current->desc_text), current->desc_buffer, (current->desc_str) ? current->desc_str : "No description");
		C2D_TextOptimize(&(current->desc_text));
	}
	free(namelist);

	start->prev = prev;

	return start;
}

void freeRomFiles(RomFile *rom_list) {
	while(rom_list) {
		free(rom_list->rom_path);
		C2D_TextBufDelete(rom_list->name_buffer);
		free(rom_list->name_str);
		free(rom_list->desc_str);
		//don't need to free C2D_Text?
		RomFile *next = rom_list->next;
		free(rom_list);
		rom_list = next;
	}
}

//menu code
int main(int argc, char* argv[]) {
	srand(time(0));
	romfsInit();
	graphics_init();
	RomFile *rom_list_start = getRomFiles();
	RomFile *selected_rom = rom_list_start;
	RomFile *selected_rom_iter;

	uint32_t keys_down = 0;

	C2D_SpriteSheet menu_bottom_t3x = C2D_SpriteSheetLoad("romfs:/gfx/menu_bottom.t3x");
	C2D_Sprite menu_bottom_bg;
	C2D_SpriteFromSheet(&menu_bottom_bg, menu_bottom_t3x, 0);

	while(aptMainLoop()) {
		hidScanInput();

		keys_down = hidKeysDown();
		if(keys_down & KEY_START) break; //exit to system
		if(keys_down & KEY_A) gameLoop(selected_rom);
		//prev loops backwards, next does not
		if(keys_down & KEY_UP && selected_rom->prev) selected_rom = selected_rom->prev;
		if(keys_down & KEY_DOWN) selected_rom = (selected_rom->next) ? selected_rom->next : rom_list_start;

		C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
		C2D_TargetClear(top_screen, off);
		C2D_SceneBegin(top_screen);

		float y = TOP_SCREEN_HEIGHT / 2.0;
		C2D_DrawText(&(selected_rom->name_text), C2D_AtBaseline | C2D_WithColor, 0.0f, y, 0.0f, 1.0f, 1.0f, on);

		y = TOP_SCREEN_HEIGHT / 2.0 - 30.0;
		selected_rom_iter = selected_rom->prev;
		while(y > 0) {
			C2D_DrawText(&(selected_rom_iter->name_text), C2D_AtBaseline | C2D_WithColor, 0.0f, y, 0.0f, 0.5f, 0.5f, on);
			selected_rom_iter = selected_rom_iter->prev;
			y -= 20.0;
		}

		y = TOP_SCREEN_HEIGHT / 2.0 + 20.0;
		selected_rom_iter = selected_rom->next;
		while(y < TOP_SCREEN_HEIGHT) {
			if(selected_rom_iter == NULL) selected_rom_iter = rom_list_start;
			C2D_DrawText(&(selected_rom_iter->name_text), C2D_AtBaseline | C2D_WithColor, 0.0f, y, 0.0f, 0.5f, 0.5f, on);
			selected_rom_iter = selected_rom_iter->next;
			y += 20.0;
		}

		C2D_TargetClear(bottom_screen, off);
		C2D_SceneBegin(bottom_screen);
		C2D_DrawSprite(&menu_bottom_bg);
		C2D_DrawText(&(selected_rom->desc_text), C2D_WithColor, 0.0f, 0.0f, 0.0f, 0.5f, 0.5f, on);


		C3D_FrameEnd(0);
	}

	freeRomFiles(rom_list_start);

	C2D_SpriteSheetFree(menu_bottom_t3x);
	graphics_exit();
	romfsExit();
	return 0;
}

