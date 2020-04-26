#include "chip8.h"

uint8_t _execute_opcode();//private

chip8_Internals internals;
uint8_t draw_flag = 0;

uint8_t chip8_drawFlag() {
	if(draw_flag) {
		draw_flag = 0;
		return 1;
	}
	return 0;
}

const uint8_t fontmap[] = {0xF0, 0x90, 0x90, 0x90, 0xF0, 0x20, 0x60, 0x20, 0x20, 0x70, 0xF0, 0x10, 0xF0, 0x80, 0xF0, 0xF0, 0x10, 0xF0, 0x10, 0xF0, 0x90, 0x90, 0xF0, 0x10, 0x10, 0xF0, 0x80, 0xF0, 0x10, 0xF0, 0xF0, 0x80, 0xF0, 0x90, 0xF0, 0xF0, 0x10, 0x20, 0x40, 0x40, 0xF0, 0x90, 0xF0, 0x90, 0xF0, 0xF0, 0x90, 0xF0, 0x10, 0xF0, 0xF0, 0x90, 0xF0, 0x90, 0x90, 0xE0, 0x90, 0xE0, 0x90, 0xE0, 0xF0, 0x80, 0x80, 0x80, 0xF0, 0xE0, 0x90, 0x90, 0x90, 0xE0, 0xF0, 0x80, 0xF0, 0x80, 0xF0, 0xF0, 0x80, 0xF0, 0x80, 0x80};

const char *chip8_load(const char *filename) {
	chip8_display_clear();
	memset(&internals, 0, sizeof(internals));

	FILE *f = fopen(filename, "rb");
	if(f == NULL) return "File doesn't exist";

	const size_t max_rom_size = sizeof(internals.memory) - PC_START;
	if(fread(internals.memory + PC_START, 1, max_rom_size+1, f) > max_rom_size) return "ROM does not fit in memory";
    fclose(f);

    memcpy(internals.memory + FONTSET_START, fontmap, sizeof(fontmap));
    internals.program_counter = PC_START;

	memset(chip8_keys_down, 0, sizeof(chip8_keys_down));
    chip8_blocking = NULL;
    draw_flag = 0;
	return NULL;
}

uint8_t chip8_cycle() {
    internals.opcode = (internals.memory[internals.program_counter] << 8) + (internals.memory[internals.program_counter + 1] & 0x00FF);
    internals.program_counter += 2;
    if(_execute_opcode()) return 1;
    return 0;
}

void chip8_decreaseTimers() {
	if(internals.delay_timer > 0) internals.delay_timer--;
    if(internals.sound_timer > 0) internals.sound_timer--;
    if(internals.sound_timer) {
        /* Beep */
    }
}

void chip8_display_clear() {
	memset(chip8_display, 0, sizeof(chip8_display));
}

uint8_t chip8_display_draw(uint8_t Vx, uint8_t Vy, uint8_t N) {
	uint8_t pixel;
	uint8_t flag = 0;
	int display_index;

	for(int y = 0; y < N; y++) {
		pixel = internals.memory[internals.I + y];

		for(int x = 0; x < 8; x++) {
			if((pixel & (0x80 >> x)) != 0) {
				display_index = (x + Vx + ((y + Vy) * CHIP_WIDTH)) / DISPLAY_BITMASK_TYPE_BITS;
				int pos = (x + Vx) % DISPLAY_BITMASK_TYPE_BITS;
				if(chip8_display[display_index] & (1 << pos)) flag = 1;
				chip8_display[display_index] ^= (1 << pos);
			}
		}

	}

	draw_flag = 1;
	return flag;
}

uint8_t _execute_opcode() {
	#define X   ((internals.opcode & 0x0F00) >> 8)
	#define Y   ((internals.opcode & 0x00F0) >> 4)
	#define N   (internals.opcode & 0x000F)
	#define NN  (internals.opcode & 0x00FF)
	#define NNN (internals.opcode & 0x0FFF)
	#define OP ((internals.opcode & 0xF000) >> 12)

	switch(OP) {
		case 0x0:
			switch(NNN) {
				case 0x000: //no-op
					break;

				case 0x0e0: //00E0
					chip8_display_clear();
					break;

				case 0x0ee: //00EE
					if(internals.stack_pointer <= 0) {
						D_LOG("E: Return from main\n");
						return 1;
					}
					internals.program_counter = internals.stack[--internals.stack_pointer];
					break;
			}
			break;

		case 0x1: //1NNN
			internals.program_counter = NNN;
			break;

		case 0x2: //2NNN
			if(internals.stack_pointer >= sizeof(internals.stack)) {
				D_LOG("E: Stack overflow\n");
				return 1;
			}

			internals.stack[internals.stack_pointer++] = internals.program_counter;
			internals.program_counter = NNN;
			break;

		case 0x3: //3XNN
			if(internals.V[X] == NN) internals.program_counter += 2;
			break;

		case 0x4: //4XNN
			if(internals.V[X] != NN) internals.program_counter += 2;
			break;

		case 0x5: //5XYN
			switch(N) {
				case 0x0:
					if(internals.V[X] == internals.V[Y]) internals.program_counter += 2;
					break;
			}
			break;

		case 0x6: //6XNN
			internals.V[X] = NN;
			break;

		case 0x7: //7XNN
			internals.V[X] += NN;
			break;

		case 0x8: //8XYN
			switch(N) {
				case 0x0:
					internals.V[X] = internals.V[Y];
					break;

				case 0x1:
					internals.V[X] |= internals.V[Y];
					break;

				case 0x2:
					internals.V[X] &= internals.V[Y];
					break;

				case 0x3:
					internals.V[X] ^= internals.V[Y];
					break;

				case 0x4: {
					int temp = internals.V[X] + internals.V[Y];
					internals.V[0xf] = (temp > 0xff);
					internals.V[X] = (uint8_t)temp;
					break;
				}

				case 0x5: {
					int temp = internals.V[X] - internals.V[Y];
					internals.V[0xf] = (temp >= 0);
					internals.V[X] = (uint8_t)temp;
					break;
				}

				case 0x6:
					internals.V[0xf] = (internals.V[X] & 0x01) ? 1 : 0;
					internals.V[X] >>= 1;
					break;

				case 0x7: {
					int temp = internals.V[Y] - internals.V[X];
					internals.V[0xf] = (temp >= 0);
					internals.V[X] = (uint8_t)temp;
					break;
				}

				case 0xE:
					internals.V[0xf] = (internals.V[X] & 0x80) ? 1 : 0;
					internals.V[X] <<= 1;
					break;
			}
			break;

		case 0x9: //9XYN
			switch(N) {
				case 0x0:
					if(internals.V[X] != internals.V[Y]) internals.program_counter += 2;
					break;
			}
			break;

		case 0xA: //ANNN
			internals.I = NNN;
			break;

		case 0xB: //BNNN
			internals.program_counter = internals.V[0] + NNN;
			break;

		case 0xC: //CXNN
			internals.V[X] = rand() & NN;
			break;

		case 0xD: //DXYN
			internals.V[0xf] = chip8_display_draw(internals.V[X], internals.V[Y], N);
			break;

		case 0xE: //EXNN
			switch(NN) {
				case 0x9e:
					if(chip8_keys_down[internals.V[X]]) internals.program_counter += 2;
					break;

				case 0xa1:
					if(!chip8_keys_down[internals.V[X]]) internals.program_counter += 2;
					break;
			}
			break;

		case 0xF: //FXNN
			switch(NN) {
				case 0x07:
					internals.V[X] = internals.delay_timer;
					break;

				case 0x0A:
					D_LOG("Waiting for input...\n");
					chip8_blocking = &internals.V[X];
					break;

				case 0x15:
					internals.delay_timer = internals.V[X];
					break;

				case 0x18:
					internals.sound_timer = internals.V[X];
					break;

				case 0x1e: {
					int temp = internals.I + internals.V[X];
					internals.V[0xf] = (temp > 0xfff);
					internals.I = (uint16_t)temp;
					break;
				}

				case 0x29:
					internals.I = FONTSET_START + internals.V[X] * 5;
					break;

				case 0x33:
					internals.memory[internals.I+0] = internals.V[X] / 100;
					internals.memory[internals.I+1] = (internals.V[X] % 100) / 10;
					internals.memory[internals.I+2] = internals.V[X] % 10;
					break;

				case 0x55:
					for(size_t i = 0; i <= X; i++) internals.memory[internals.I + i] = internals.V[i];
					break;

				case 0x65:
					for(size_t i = 0; i <= X; i++) internals.V[i] = internals.memory[internals.I + i];
					break;
			}
			break;

		default:
			D_LOG("unimplemented opcode - %04x\n", internals.opcode);
			break;
	}

	return 0;
}

#ifdef DEBUG

//debugging
void chip8_printInternals() {
	printf("program_counter - 0x%04x\n", internals.program_counter);
	printf("index - %04x\n", internals.I);
	printf("opcode - 0x%04x\n", internals.opcode);
	puts("       0x0 0x1 0x2 0x3 0x4 0x5 0x6 0x7 0x8 0x9 0xA 0xB 0xC 0xD 0xE 0xF");

	printf("Keys - ");
	for(int i = 0; i < sizeof(chip8_keys_down); i++) {
		printf("%03x ", chip8_keys_down[i]);
	}

	printf("\n");

	printf("Regs - ");
	for(int i = 0; i < sizeof(internals.V); i++) {
		printf("%03x ", internals.V[i]);
	}

	printf("\n\n");
}

void chip8_memoryDump() {
	FILE *f = fopen("mem_dump", "wb");
	fwrite(internals.memory, 1, sizeof(internals.memory), f);
	fclose(f);
}

#endif
