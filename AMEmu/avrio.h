#pragma once

#include "avrcore.h"

void IOWriteDefault(AVRMCU *u, uint8_t addr, uint8_t data);
uint8_t IOReadDefault(AVRMCU *u, uint8_t addr);
void IOWriteReserved(AVRMCU *u, uint8_t addr, uint8_t data);
uint8_t IOReadReserved(AVRMCU *u, uint8_t addr);

extern const IOInitCallback IOInitCallbackDefault[0x100 - 0x20];
extern const IOWriteCallback IOWriteCallbackDefault[0x100 - 0x20];
extern const IOReadCallback IOReadCallbackDefault[0x100 - 0x20];
