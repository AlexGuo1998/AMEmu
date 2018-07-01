#pragma once

#include <stdint.h>

typedef struct AVRMCU_t {
	uint16_t pc;
	uint16_t rom[16 * 1024];//32kB, 16k words
	uint8_t sram[0x100 + 2560];//reg + IO + SRAM
	uint8_t eeprom[1024];//1kB
	void(*iowrite[0x100 - 0x20])(struct AVRMCU_t *u, uint8_t addr, uint16_t data);//can be NULL for write-through
	uint8_t(*ioread[0x100 - 0x20])(struct AVRMCU_t *u, uint8_t addr);
} AVRMCU;

typedef void(*IOWriteCallback)(AVRMCU *u, uint8_t addr, uint8_t data);
typedef uint8_t(*IOReadCallback)(AVRMCU *u, uint8_t addr);
