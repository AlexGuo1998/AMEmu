#pragma once

#include "avrcore.h"

void IOWriteDefault(AVRMCU *u, uint8_t addr, uint8_t data);
uint8_t IOReadDefault(AVRMCU *u, uint8_t addr);
void IOWriteReserved(AVRMCU *u, uint8_t addr, uint8_t data);
uint8_t IOReadReserved(AVRMCU *u, uint8_t addr);
