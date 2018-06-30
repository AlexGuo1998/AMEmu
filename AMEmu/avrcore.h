#pragma once

#include <stdint.h>

typedef struct AVRMCU_t {
	uint16_t pc;
	uint16_t rom[16 * 1024];//32kB, 16k words
	uint8_t reg[256];//reg + io port
	uint8_t sram[2560];//2.5kB
	uint8_t eeprom[1024];//1kB
} AVRMCU;
