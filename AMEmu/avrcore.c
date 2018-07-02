#include "avrcore.h"

#include <assert.h>
#include <stdbool.h>

#define TestInst2Word(inst) (\
	(((inst) & 0b1111111000001100) == 0b1001010000001100)/*jmp, call*/ || \
	(((inst) & 0b1111110000001111) == 0b1001000000000000)/*lds, sts*/ \
)

#define SREG 0x5F
#pragma region SREG get/set/clr macros

#define SREG_C 0
#define SREG_Z 1
#define SREG_N 2
#define SREG_V 3
#define SREG_S 4
#define SREG_H 5
#define SREG_T 6
#define SREG_I 7

#define GETC() (u->sram[SREG] & (1 << SREG_C))
#define GETZ() (u->sram[SREG] & (1 << SREG_Z))
#define GETN() (u->sram[SREG] & (1 << SREG_N))
#define GETV() (u->sram[SREG] & (1 << SREG_V))
#define GETS() (u->sram[SREG] & (1 << SREG_S))
#define GETH() (u->sram[SREG] & (1 << SREG_H))
#define GETT() (u->sram[SREG] & (1 << SREG_T))
#define GETI() (u->sram[SREG] & (1 << SREG_I))

#define SETC(x) (u->sram[SREG] = (u->sram[SREG] & ~(1 << SREG_C)) | ((x) ? 1 << SREG_C : 0))
#define SETZ(x) (u->sram[SREG] = (u->sram[SREG] & ~(1 << SREG_Z)) | ((x) ? 1 << SREG_Z : 0))
#define SETN(x) (u->sram[SREG] = (u->sram[SREG] & ~(1 << SREG_N)) | ((x) ? 1 << SREG_N : 0))
#define SETV(x) (u->sram[SREG] = (u->sram[SREG] & ~(1 << SREG_V)) | ((x) ? 1 << SREG_V : 0))
#define SETS(x) (u->sram[SREG] = (u->sram[SREG] & ~(1 << SREG_S)) | ((x) ? 1 << SREG_S : 0))
#define SETH(x) (u->sram[SREG] = (u->sram[SREG] & ~(1 << SREG_H)) | ((x) ? 1 << SREG_H : 0))
#define SETT(x) (u->sram[SREG] = (u->sram[SREG] & ~(1 << SREG_T)) | ((x) ? 1 << SREG_T : 0))
#define SETI(x) (u->sram[SREG] = (u->sram[SREG] & ~(1 << SREG_I)) | ((x) ? 1 << SREG_I : 0))

#pragma endregion
#define SPL 0x5D
#define SPH 0x5E

#define REGX 0x1A
#define REGY 0x1C
#define REGZ 0x1E

#define LOW(data) ((data) & 0xFF)
#define HIGH(data) (((data) >> 8) & 0xFF)


#define CheckSRAMAddr(addr) ((uint16_t)(addr) <= 0xAFF)
#define CheckROMAddr(addr) ((uint16_t)(addr) <= (32 * 1024))

inline static bool MemWrite(AVRMCU *u, uint16_t addr, uint8_t data) {
	if (addr >= 0x20 && addr <= 0xFF && u->iowrite[addr - 0x20] != NULL) {
		//IO callback
		u->iowrite[addr - 0x20](u, (uint8_t)addr, data);
	} else {
		//direct
		if (!CheckSRAMAddr(addr)) {
			return false;
		}
		u->sram[addr] = data;
	}
	return true;
}

inline static bool MemRead(AVRMCU *u, uint16_t addr, uint8_t *data) {
	if (addr >= 0x20 && addr <= 0xFF && u->ioread[addr - 0x20] != NULL) {
		//IO callback
		*data = u->ioread[addr - 0x20](u, (uint8_t)addr);
	} else {
		//direct
		if (!CheckSRAMAddr(addr)) {
			return false;
		}
		*data = u->sram[addr];
	}
	return true;
}

inline static bool RomRead(AVRMCU *u, uint16_t addr, uint8_t *data) {
	if (!CheckROMAddr(addr)) {
		return false;
	}
	*data = (u->rom[addr >> 1] >> ((addr & 1) ? 8 : 0)) & 0xFF;
	return true;
}

inline static bool PushPC(AVRMCU *u) {
	assert(u->ioread[SPL - 0x20] == NULL);
	assert(u->ioread[SPH - 0x20] == NULL);
	assert(u->iowrite[SPL - 0x20] == NULL);
	assert(u->iowrite[SPH - 0x20] == NULL);
	uint16_t sp = (u->sram[SPH] << 8) | u->sram[SPL];
	//WARNING: Wrong operation when SP -> SP
	//push
	bool ret = MemWrite(u, sp--, u->pc & 0xFF);
	ret = ret && MemWrite(u, sp--, u->pc >> 8);
	//write back sp
	u->sram[SPL] = sp & 0xFF;
	u->sram[SPH] = sp >> 8;
	return ret;
}

int avr_runstep(AVRMCU *u) {
	assert(u->ioread[SREG - 0x20] == NULL);
	assert(u->iowrite[SREG - 0x20] == NULL);
	//TODO test PC wraparound?
	int cycle = 1;
	uint16_t inst = u->rom[u->pc];
	u->pc++;
	//process operation
	if (inst == 0) {
		//nop
	} else if ((inst & 0xFC00) == 0x0000) {
		if ((inst & 0x0300) == 0x0300) {
			//mulsu, fmul, fmuls, fmulsu
			cycle++;//2 cycles
			uint8_t rd, rr;
			rd = ((inst & 0x0070) >> 4) | 0x10;
			rr = (inst & 0x0007) | 0x10;
			if ((inst & 0x0080) == 0x0000) {
				if ((inst & 0x0008) == 0x0000) {
					//mulsu
					{
						int8_t d = (int8_t)u->sram[rd];
						uint8_t r = u->sram[rr];
						int32_t o = d * r;
						SETZ(o == 0);
						SETC(o & (1 << 15));
						u->sram[0] = LOW(o);
						u->sram[1] = HIGH(o);
					}
				} else {
					//fmul
					{
						uint8_t d = u->sram[rd];
						uint8_t r = u->sram[rr];
						uint32_t o = d * r;
						SETZ(o == 0);
						SETC(o & (1 << 15));
						o <<= 1;
						u->sram[0] = LOW(o);
						u->sram[1] = HIGH(o);
					}
				}
			} else {
				if ((inst & 0x0008) == 0x0000) {
					//fmuls
					{
						int8_t d = (int8_t)u->sram[rd];
						int8_t r = (int8_t)u->sram[rr];
						int32_t o = d * r;
						SETZ(o == 0);
						SETC(o & (1 << 15));
						o <<= 1;
						u->sram[0] = LOW(o);
						u->sram[1] = HIGH(o);
					}
				} else {
					//fmulsu
					{
						int8_t d = (int8_t)u->sram[rd];
						uint8_t r = u->sram[rr];
						int32_t o = d * r;
						SETZ(o == 0);
						SETC(o & (1 << 15));
						o <<= 1;
						u->sram[0] = LOW(o);
						u->sram[1] = HIGH(o);
					}
				}
			}
		} else if ((inst & 0x0300) == 0x0100) {
			//movw
			{
				uint8_t rd, rr;
				rd = (inst & 0x00F0) >> 3;
				rr = (inst & 0x000F) << 1;
				u->sram[rd] = u->sram[rr];
				u->sram[rd + 1] = u->sram[rr + 1];
			}
		} else if ((inst & 0x0300) == 0x0200) {
			//muls
			{
				uint8_t rd, rr;
				rd = ((inst & 0x00F0) >> 4) | 0x10;
				rr = (inst & 0x000F) | 0x10;

				int8_t d = (int8_t)u->sram[rd];
				int8_t r = (int8_t)u->sram[rr];
				int32_t o = d * r;
				SETZ(o == 0);
				SETC(o & (1 << 15));
				u->sram[0] = LOW(o);
				u->sram[1] = HIGH(o);
			}
		} else {
			//wrong op
			cycle = -1;
		}
	} else if ((((inst & 0xC000) == 0x0000) && ((inst & 0x3000) != 0x3000)) || ((inst & 0xFC00) == 0x9C00)) {
		uint8_t rd, rr;
		rd = (inst >> 4) & 0x1F;
		rr = (inst & 0x000F) | ((inst >> 5) & 0x0010);
		uint8_t d, r;
		d = u->sram[rd];
		r = u->sram[rr];
		if ((inst & 0x8000) == 0x0000) {
			switch (inst >> 10) {
				case 0x01:
					//cpc
					//TODO: (cpc) need test!
					{
						uint8_t o, ctest, vtest;
						o = d - r - (GETC() ? 1 : 0);
						ctest = (~d & r) | (r & o) | (o & ~d);
						vtest = (d & ~r & ~o) | (~d & r & o);
						SETC(ctest & (1 << 7));
						if (o != 0) SETZ(0);
						SETN(o & (1 << 7));
						SETV(vtest & (1 << 7));
						SETS((o ^ vtest) & (1 << 7));
						SETH(ctest & (1 << 3));
					}
					break;
				case 0x02:
					//sbc
					{
						uint8_t o, ctest, vtest;
						o = d - r - (GETC() ? 1 : 0);
						ctest = (~d & r) | (r & o) | (o & ~d);
						vtest = (d & ~r & ~o) | (~d & r & o);
						SETC(ctest & (1 << 7));
						if (o != 0) SETZ(0);
						SETN(o & (1 << 7));
						SETV(vtest & (1 << 7));
						SETS((o ^ vtest) & (1 << 7));
						SETH(ctest & (1 << 3));
						u->sram[rd] = o;
					}
					break;
				case 0x03:
					//add
					{
						uint8_t o, ctest, vtest;
						o = d + r;
						ctest = (d & r) | (r & ~o) | (~o & d);
						vtest = (d & r & ~o) | (~d & ~r & o);
						SETC(ctest & (1 << 7));
						SETZ(o == 0);
						SETN(o & (1 << 7));
						SETV(vtest & (1 << 7));
						SETS((o ^ vtest) & (1 << 7));
						SETH(ctest & (1 << 3));
						u->sram[rd] = o;
					}
					break;
				case 0x04:
					//cpse
					{
						uint8_t o = d - r;
						if (o == 0) {
							//equal, do skip
							uint16_t inst1 = u->rom[u->pc];
							if (TestInst2Word(inst1)) {
								cycle++;
								u->pc++;
							}
							cycle++;
							u->pc++;
						}
					}
					break;
				case 0x05:
					//cp
					{
						uint8_t o, ctest, vtest;
						o = d - r;
						ctest = (~d & r) | (r & o) | (o & ~d);
						vtest = (d & ~r & ~o) | (~d & r & o);
						SETC(ctest & (1 << 7));
						SETZ(o == 0);
						SETN(o & (1 << 7));
						SETV(vtest & (1 << 7));
						SETS((o ^ vtest) & (1 << 7));
						SETH(ctest & (1 << 3));
					}
					break;
				case 0x06:
					//sub
					{
						uint8_t o, ctest, vtest;
						o = d - r;
						ctest = (~d & r) | (r & o) | (o & ~d);
						vtest = (d & ~r & ~o) | (~d & r & o);
						SETC(ctest & (1 << 7));
						SETZ(o == 0);
						SETN(o & (1 << 7));
						SETV(vtest & (1 << 7));
						SETS((o ^ vtest) & (1 << 7));
						SETH(ctest & (1 << 3));
						u->sram[rd] = o;
					}
					break;
				case 0x07:
					//adc
					{
						uint8_t o, ctest, vtest;
						o = d + r + (GETC() ? 1 : 0);
						ctest = (d & r) | (r & ~o) | (~o & d);
						vtest = (d & r & ~o) | (~d & ~r & o);
						SETC(ctest & (1 << 7));
						SETZ(o == 0);
						SETN(o & (1 << 7));
						SETV(vtest & (1 << 7));
						SETS((o ^ vtest) & (1 << 7));
						SETH(ctest & (1 << 3));
						u->sram[rd] = o;
					}
					break;
				case 0x08:
					//and
					{
						uint8_t o = d & r;
						SETZ(o == 0);
						SETN(o & (1 << 7));
						SETV(0);
						SETS(GETN() ^ 0);
						u->sram[rd] = o;
					}
					break;
				case 0x09:
					//xor
					{
						uint8_t o = d ^ r;
						SETZ(o == 0);
						SETN(o & (1 << 7));
						SETV(0);
						SETS(GETN() ^ 0);
						u->sram[rd] = o;
					}
					break;
				case 0x0A:
					//or
					{
						uint8_t o = d | r;
						SETZ(o == 0);
						SETN(o & (1 << 7));
						SETV(0);
						SETS(GETN() ^ 0);
						u->sram[rd] = o;
					}
					break;
				case 0x0B:
					//mov
					{
						u->sram[rd] = r;
					}
					break;
				default:
					assert(false);
			}
		} else {
			//mul
			{
				uint32_t o = d * r;
				SETZ(o == 0);
				SETC(o & (1 << 15));
				u->sram[0] = LOW(o);
				u->sram[1] = HIGH(o);
			}
		}
	} else if (((inst & 0x8000) == 0x0000) || ((inst & 0xF000) == 0xE000)) {
		uint8_t rd;
		rd = ((inst >> 4) & 0x0F) | 0x10;
		uint8_t d, r;
		d = u->sram[rd];
		r = (inst & 0x0F) | ((inst >> 4) & 0xF0);
		switch (inst >> 12) {
			case 0x3:
				//cpi
				{
					uint8_t o, ctest, vtest;
					o = d - r;
					ctest = (~d & r) | (r & o) | (o & ~d);
					vtest = (d & ~r & ~o) | (~d & r & o);
					SETC(ctest & (1 << 7));
					SETZ(o == 0);
					SETN(o & (1 << 7));
					SETV(vtest & (1 << 7));
					SETS((o ^ vtest) & (1 << 7));
					SETH(ctest & (1 << 3));
				}
				break;
			case 0x4:
				//sbci
				{
					uint8_t o, ctest, vtest;
					o = d - r - (GETC() ? 1 : 0);
					ctest = (~d & r) | (r & o) | (o & ~d);
					vtest = (d & ~r & ~o) | (~d & r & o);
					SETC(ctest & (1 << 7));
					if (o != 0) SETZ(0);
					SETN(o & (1 << 7));
					SETV(vtest & (1 << 7));
					SETS((o ^ vtest) & (1 << 7));
					SETH(ctest & (1 << 3));
					u->sram[rd] = o;
				}
				break;
			case 0x5:
				//subi
				{
					uint8_t o, ctest, vtest;
					o = d - r;
					ctest = (~d & r) | (r & o) | (o & ~d);
					vtest = (d & ~r & ~o) | (~d & r & o);
					SETC(ctest & (1 << 7));
					SETZ(o == 0);
					SETN(o & (1 << 7));
					SETV(vtest & (1 << 7));
					SETS((o ^ vtest) & (1 << 7));
					SETH(ctest & (1 << 3));
					u->sram[rd] = o;
				}
				break;
			case 0x6:
				//ori
				{
					uint8_t o = d | r;
					SETZ(o == 0);
					SETN(o & (1 << 7));
					SETV(0);
					SETS(GETN() ^ 0);
					u->sram[rd] = o;
				}
				break;
			case 0x7:
				//andi
				{
					uint8_t o = d & r;
					SETZ(o == 0);
					SETN(o & (1 << 7));
					SETV(0);
					SETS(GETN() ^ 0);
					u->sram[rd] = o;
				}
				break;
			case 0xE:
				//ldi
				{
					u->sram[rd] = r;
				}
				break;
			default:
				assert(false);
		}
	} else if ((inst & 0xC000) == 0x8000) {
		//10xx xxxx xxxx xxxx
		uint8_t rd = (inst >> 4) & 0x1F;
		uint8_t d = u->sram[rd];
		if ((inst & 0x1000) == 0x1000) {
			if ((inst & 0x2000) == 0x2000) {
				//in, out
				uint8_t io = (inst & 0x000F) | ((inst >> 5) & 0x0030);
				uint8_t addr = io + 0x20;
				bool ret;
				if ((inst & 0x0800) == 0x0800) {
					//out
					ret = MemWrite(u, addr, d);
				} else {
					//in
					ret = MemRead(u, addr, &u->sram[rd]);
				}
				assert(ret);
			} else {
				//1001 xxxx xxxx xxxx
				switch ((inst >> 10) & 0x0003) {
					case 0x0:
						{
							cycle++;
							bool ret;
							uint16_t addr;
							switch (inst & 0x000F) {
								case 0x0:
									//lds, sts
									{
										addr = u->rom[u->pc];
										u->pc++;
										if ((inst & 0x0200) == 0x0200) {
											//st
											ret = MemWrite(u, addr, d);
										} else {
											//ld
											ret = MemRead(u, addr, &u->sram[rd]);
										}
										if (!ret) {
											cycle = -1;
										}
									}
									break;
								case 0x1:
									//ld, st Z+
									{
										addr = ((uint16_t)u->sram[REGZ + 1] << 8) | u->sram[REGZ];
										if ((inst & 0x0200) == 0x0200) {
											//st
											ret = MemWrite(u, addr, d);
										} else {
											//ld
											ret = MemRead(u, addr, &u->sram[rd]);
										}
										if (!ret) {
											cycle = -1;
										}
										addr++;
										u->sram[REGZ] = addr & 0xFF;
										u->sram[REGZ + 1] = addr >> 8;
									}
									break;
								case 0x2:
									//ld, st -Z
									{
										addr = ((uint16_t)u->sram[REGZ + 1] << 8) | u->sram[REGZ];
										addr--;
										if ((inst & 0x0200) == 0x0200) {
											//st
											ret = MemWrite(u, addr, d);
										} else {
											//ld
											ret = MemRead(u, addr, &u->sram[rd]);
										}
										if (!ret) {
											cycle = -1;
										}
										u->sram[REGZ] = addr & 0xFF;
										u->sram[REGZ + 1] = addr >> 8;
									}
									break;
								case 0x3:
									//wrong op
									{
										cycle = -1;
									}
									break;
								case 0x4:
									//lpm, xch
									{
										addr = ((uint16_t)u->sram[REGZ + 1] << 8) | u->sram[REGZ];
										if ((inst & 0x0200) == 0x0200) {
											//xch
											ret = MemRead(u, addr, &u->sram[rd]);
											ret = ret && MemWrite(u, addr, d);
										} else {
											//lpm
											cycle++;
											ret = RomRead(u, addr, &u->sram[rd]);
										}
										if (!ret) {
											cycle = -1;
										}
									}
									break;
								case 0x5:
									//lpm Z+, las
									{
										addr = ((uint16_t)u->sram[REGZ + 1] << 8) | u->sram[REGZ];
										if ((inst & 0x0200) == 0x0200) {
											//las
											ret = MemRead(u, addr, &u->sram[rd]);
											ret = ret && MemWrite(u, addr, u->sram[rd] | d);
										} else {
											//lpm Z+
											cycle++;
											ret = RomRead(u, addr, &u->sram[rd]);
											addr++;
											u->sram[REGZ] = addr & 0xFF;
											u->sram[REGZ + 1] = addr >> 8;
										}
										if (!ret) {
											cycle = -1;
										}
									}
									break;
								case 0x6:
									//lac
									{
										addr = ((uint16_t)u->sram[REGZ + 1] << 8) | u->sram[REGZ];
										if ((inst & 0x0200) == 0x0200) {
											//lac
											ret = MemRead(u, addr, &u->sram[rd]);
											ret = ret && MemWrite(u, addr, u->sram[rd] & ~d);
										} else {
											//wrong op
											ret = false;
										}
										if (!ret) {
											cycle = -1;
										}
									}
									break;
								case 0x7:
									//lat
									{
										addr = ((uint16_t)u->sram[REGZ + 1] << 8) | u->sram[REGZ];
										if ((inst & 0x0200) == 0x0200) {
											//lat
											ret = MemRead(u, addr, &u->sram[rd]);
											ret = ret && MemWrite(u, addr, u->sram[rd] ^ d);
										} else {
											//wrong op
											ret = false;
										}
										if (!ret) {
											cycle = -1;
										}
									}
									break;
								case 0x8:
									//wrong op
									{
										cycle = -1;
									}
									break;
								case 0x9:
									//ld, st Y+
									{
										addr = ((uint16_t)u->sram[REGY + 1] << 8) | u->sram[REGY];
										if ((inst & 0x0200) == 0x0200) {
											//st
											ret = MemWrite(u, addr, d);
										} else {
											//ld
											ret = MemRead(u, addr, &u->sram[rd]);
										}
										if (!ret) {
											cycle = -1;
										}
										addr++;
										u->sram[REGY] = addr & 0xFF;
										u->sram[REGY + 1] = addr >> 8;
									}
									break;
								case 0xA:
									//ld, st -Y
									{
										addr = ((uint16_t)u->sram[REGY + 1] << 8) | u->sram[REGY];
										addr--;
										if ((inst & 0x0200) == 0x0200) {
											//st
											ret = MemWrite(u, addr, d);
										} else {
											//ld
											ret = MemRead(u, addr, &u->sram[rd]);
										}
										if (!ret) {
											cycle = -1;
										}
										u->sram[REGY] = addr & 0xFF;
										u->sram[REGY + 1] = addr >> 8;
									}
									break;
								case 0xB:
									//wrong op
									{
										cycle = -1;
									}
									break;
								case 0xC:
									//ld, st X
									{
										addr = ((uint16_t)u->sram[REGX + 1] << 8) | u->sram[REGX];
										if ((inst & 0x0200) == 0x0200) {
											//st
											ret = MemWrite(u, addr, d);
										} else {
											//ld
											ret = MemRead(u, addr, &u->sram[rd]);
										}
										if (!ret) {
											cycle = -1;
										}
									}
									break;
								case 0xD:
									//ld, st X+
									{
										addr = ((uint16_t)u->sram[REGX + 1] << 8) | u->sram[REGX];
										if ((inst & 0x0200) == 0x0200) {
											//st
											ret = MemWrite(u, addr, d);
										} else {
											//ld
											ret = MemRead(u, addr, &u->sram[rd]);
										}
										if (!ret) {
											cycle = -1;
										}
										addr++;
										u->sram[REGX] = addr & 0xFF;
										u->sram[REGX + 1] = addr >> 8;
									}
									break;
								case 0xE:
									//ld, st -X
									{
										addr = ((uint16_t)u->sram[REGX + 1] << 8) | u->sram[REGX];
										addr--;
										if ((inst & 0x0200) == 0x0200) {
											//st
											ret = MemWrite(u, addr, d);
										} else {
											//ld
											ret = MemRead(u, addr, &u->sram[rd]);
										}
										if (!ret) {
											cycle = -1;
										}
										u->sram[REGX] = addr & 0xFF;
										u->sram[REGX + 1] = addr >> 8;
									}
									break;
								case 0xF:
									//pop, push
									{
										assert(u->ioread[SPL - 0x20] == NULL);
										assert(u->ioread[SPH - 0x20] == NULL);
										assert(u->iowrite[SPL - 0x20] == NULL);
										assert(u->iowrite[SPH - 0x20] == NULL);
										uint16_t sp = (u->sram[SPH] << 8) | u->sram[SPL];
										if ((inst & 0x0200) == 0x0200) {
											//push
											ret = MemWrite(u, sp--, d);
										} else {
											//pop
											ret = MemRead(u, ++sp, &u->sram[rd]);
										}
										if (!ret) {
											cycle = -1;
										}
										//write back sp
										u->sram[SPL] = sp & 0xFF;
										u->sram[SPH] = sp >> 8;
									}
									break;
								default:
									assert(false);
							}
						}
						break;
					case 0x1:
						{
							//1001 01xx xxxx xxxx
							if ((inst & 0x0200) == 0x0000) {
								//1001 010x xxxx xxxx
								switch (inst & 0x000F) {
									case 0x0:
										//com
										{
											uint8_t o = 0xFF - d;
											SETC(1);
											SETZ(o == 0);
											SETN(o & (1 << 7));
											SETV(0);
											SETS(GETN() ^ 0);
											u->sram[rd] = o;
										}
										break;
									case 0x1:
										//neg
										{
											uint8_t o = 0 - d;
											SETC(o != 0);
											SETZ(o == 0);
											SETN(o & (1 << 7));
											SETV(o == 0x80);
											SETS(o > 0x80);
											SETH((o & 0x0F) == 0x00);
											u->sram[rd] = o;
										}
										break;
									case 0x2:
										//swap
										{
											uint8_t o = ((d & 0xF0) >> 4) | ((d & 0x0F) << 4);
											u->sram[rd] = o;
										}
										break;
									case 0x3:
										//inc
										{
											uint8_t o = d + 1;
											SETZ(o == 0);
											SETN(o & (1 << 7));
											SETV(o == 0x80);
											SETS(o > 0x80);
											u->sram[rd] = o;
										}
										break;
									case 0x4:
										//wrong op
										cycle = -1;
										break;
									case 0x5:
										//asr
										{
											uint8_t o = (uint8_t)((int8_t)d / 2);
											assert((o & 0x80) == (d & 0x80));
											SETC(d & (1 << 0));
											SETZ(o == 0);
											SETN(o & (1 << 7));
											SETV((d & 1) ^ ((o >> 7) & 1));//V = N ^ C
											SETS(d & (1 << 0));//S = N ^ V = C
											u->sram[rd] = o;
										}
										break;
									case 0x6:
										//lsr
										{
											uint8_t o = d >> 1;
											SETC(d & (1 << 0));
											SETZ(o == 0);
											SETN(0);
											SETV(d & (1 << 0));//V = N ^ C = C
											SETS(d & (1 << 0));//S = N ^ V = C
											u->sram[rd] = o;
										}
										break;
									case 0x7:
										//ror
										{
											uint8_t o = d >> 1 | (GETC() ? (1 << 7) : 0);
											SETZ(o == 0);
											SETN(GETC());
											SETV(GETC() ^ (d & (1 << 0)));//V = N ^ C
											SETC(d & (1 << 0));
											SETS(d & (1 << 0));//S = N ^ V = C
											u->sram[rd] = o;
										}
										break;
									case 0x8:
										{
											if ((inst & 0x0100) == 0x0100) {
												bool returnfrominterrupt = false;
												switch ((inst >> 4) & 0xF) {
													case 0x1:
														//reti
														returnfrominterrupt = true;
														SETI(1);
														//fall-through
													case 0x0:
														//ret
														{
															cycle += 3;
															assert(u->ioread[SPL - 0x20] == NULL);
															assert(u->ioread[SPH - 0x20] == NULL);
															assert(u->iowrite[SPL - 0x20] == NULL);
															assert(u->iowrite[SPH - 0x20] == NULL);
															uint16_t sp = (u->sram[SPH] << 8) | u->sram[SPL];
															uint8_t pch, pcl;
															bool ret = MemRead(u, ++sp, &pch);
															ret = ret && MemRead(u, ++sp, &pcl);
															if (ret) {
																u->pc = ((uint16_t)pch << 8) | pcl;
															} else {
																cycle = -1;
															}
															//write back sp
															u->sram[SPL] = sp & 0xFF;
															u->sram[SPH] = sp >> 8;
															if (returnfrominterrupt && cycle > 0) {
																//run one more step
																int cycle1 = avr_runstep(u);
																if (cycle1 < 0) {
																	//error
																	cycle = cycle1;
																} else {
																	cycle += cycle1;
																}
															}
														}
														break;
													case 0x2:
													case 0x3:
													case 0x4:
													case 0x5:
													case 0x6:
													case 0x7:
														//wrong op
														cycle = -1;
														break;
													case 0x8:
														//sleep
														//TODO sleep
														break;
													case 0x9:
														//break
														//TODO break
														break;
													case 0xA:
														//wdr
														//TODO wdt reset
														break;
													case 0xB:
														//wrong op
														cycle = -1;
														break;
													case 0xC:
														//lpm
														{
															rd = 0;//use r0
															uint16_t addr = ((uint16_t)u->sram[REGZ + 1] << 8) | u->sram[REGZ];
															cycle += 2;
															bool ret = RomRead(u, addr, &u->sram[rd]);
															if (!ret) {
																cycle = -1;
															}
														}
														break;
													case 0xD:
														//wrong op
														cycle = -1;
														break;
													case 0xE:
														//spm
														//TODO spm
														break;
													case 0xF:
														//wrong op
														cycle = -1;
														break;
													default:
														assert(false);
												}
											} else {
												//bset, bclr
												uint8_t bit = (inst >> 4) & 0x7;
												if ((inst & 0x0080) == 0x0080) {
													//bclr
													u->sram[SREG] &= ~(1 << bit);
												} else {
													//bset
													u->sram[SREG] |= (1 << bit);
												}
											}
										}
										break;
									case 0x9:
										//ijmp. icall
										{
											uint16_t addr = ((uint16_t)u->sram[REGZ + 1] << 8) | u->sram[REGZ];
											if (inst == 0x9409) {
												//ijmp
												cycle++;//2
												u->pc = addr;
											} else if (inst == 0x9509) {
												//icall
												cycle += 2;//3
												if (!PushPC(u)) {
													//SP Out of range error
													cycle = -1;
												}
												u->pc = addr;
											} else {
												//wrong op
												cycle = -1;
											}
										}
										break;
									case 0xA:
										//dec
										{
											uint8_t o = d - 1;
											SETZ(o == 0);
											SETN(o & (1 << 7));
											SETV(o == 0x7F);
											SETS(o >= 0x7F);
											u->sram[rd] = o;
										}
										break;
									case 0xB:
										//wrong op
										cycle = -1;
										break;
									case 0xC:
									case 0xD:
										//jmp
										{
											cycle += 2;//3
											if (inst & 0x01F1 != 0x0000) {
												//pc out of range
												cycle = -1;
											}
											uint16_t addr = u->rom[u->pc];
											u->pc = addr;
										}
										break;
									case 0xE:
									case 0xF:
										//call
										{
											cycle += 3;//4
											if (inst & 0x01F1 != 0x0000) {
												//pc out of range
												cycle = -1;
											}
											uint16_t addr = u->rom[u->pc];
											u->pc++;
											if (!PushPC(u)) {
												//SP Out of range error
												cycle = -1;
											}
											u->pc = addr;
										}
										break;
									default:
										assert(false);
								}
							} else {
								//adiw, sbiw
								cycle++;
								rd = ((inst >> 3) & 0x06) | 0x18;
								uint8_t k = (inst & 0x0F) | ((inst >> 2) & 0x30);
								uint16_t d16 = ((uint16_t)u->sram[rd + 1] << 8) | u->sram[rd];
								uint16_t o16, ctest16, vtest16;
								if ((inst & 0x0100) == 0x0100) {
									//sbiw
									o16 = d16 - k;
									ctest16 = ~d16 & o16;
								} else {
									//adiw
									o16 = d16 + k;
									ctest16 = ~o16 & d16;
								}
								vtest16 = ~d16 & o16;
								SETC(ctest16 & (1 << 15));
								SETZ(o16 == 0);
								SETN(o16 & (1 << 15));
								SETV(vtest16 & (1 << 15));
								SETS((o16 ^ vtest16) & (1 << 15));
								u->sram[rd] = d16 & 0xFF;
								u->sram[rd + 1] = d16 >> 8;
							}
						}
						break;
					case 0x2:
						//cbi, sbi, sbic, sbis
						{
							uint8_t io = (inst >> 3) & 0x001F;
							uint8_t addr = io + 0x20;
							uint8_t bit = inst & 0x0007;
							uint8_t data;
							bool ret = MemRead(u, addr, &data);
							assert(ret);
							if ((inst & 0x0100) == 0x0000) {
								//cbi, sbi
								cycle++;
								if ((inst & 0x0200) == 0x0200) {
									//sbi
									data |= 1 << bit;
								} else {
									//cbi
									data &= ~(1 << bit);
								}
								ret = MemWrite(u, addr, data);
								assert(ret);
							} else {
								//sbic, sbis
								if (((data >> bit) & 1) == ((inst >> 9) & 1)) {
									uint16_t inst1 = u->rom[u->pc];
									if (TestInst2Word(inst1)) {
										cycle++;
										u->pc++;
									}
									cycle++;
									u->pc++;
								}
							}
						}
						break;
					case 0x3:
						//wrong op
						{
							cycle = -1;
						}
						break;
					default:
						assert(false);
				}
			}
		} else {
			//ld(d), st(d)
			cycle++;
			uint8_t delta = (inst & 0x07) | ((inst >> 7) & 0x18) | ((inst >> 8) & 0x20);
			uint16_t addr;
			if ((inst & 0x0008) == 0x0008) {
				//use Y
				addr = ((uint16_t)u->sram[REGY + 1] << 8) | u->sram[REGY];
			} else {
				//use Z
				addr = ((uint16_t)u->sram[REGZ + 1] << 8) | u->sram[REGZ];
			}
			addr += delta;
			bool ret;
			if ((inst & 0x0200) == 0x0200) {
				//st
				ret = MemWrite(u, addr, d);
			} else {
				//ld
				ret = MemRead(u, addr, &u->sram[rd]);
			}
			if (!ret) {
				cycle = -1;
			}
		}
	} else if ((inst & 0xC000) == 0xC000) {
		//11xx xxxx xxxx xxxx
		if ((inst & 0xE000) == 0xC000) {
			//rjmp, rcall
			cycle++;//2 for rjmp
			int16_t delta = inst & 0x0FFF;
			if ((delta & 0x0800) == 0x0800) {// < 0
				delta |= 0xF000;
			}
			if ((inst & 0x1000) == 0x1000) {
				cycle++;//3 for rcall
				if (!PushPC(u)) {
					//SP Out of range error
					cycle = -1;
				}
			}
			u->pc += delta;
		} else {
			//111x xxxx xxxx xxxx
			if ((inst & 0x1000) == 0x1000) {
				//1111 xxxx xxxx xxxx
				uint8_t bit = inst & 0x07;
				if ((inst & 0x0800) == 0x0800) {
					uint8_t rd = (inst >> 4) & 0x1F;
					uint8_t d = u->sram[rd];
					if ((inst & 0x0008) == 0x0000) {
						switch ((inst >> 9) & 0x03) {
							case 0x00:
								//bld
								u->sram[rd] = (rd & ~(1 << bit)) | (GETT() ? 1 << bit : 0);
								break;
							case 0x01:
								//bst
								SETT(rd & (1 << bit));
								break;
							case 0x02:
								//sbrc
								{
									if (!(d & (1 << bit))) {
										uint16_t inst1 = u->rom[u->pc];
										if (TestInst2Word(inst1)) {
											cycle++;
											u->pc++;
										}
										cycle++;
										u->pc++;
									}
								}
								break;
							case 0x03:
								//sbrs
								{
									if (d & (1 << bit)) {
										uint16_t inst1 = u->rom[u->pc];
										if (TestInst2Word(inst1)) {
											cycle++;
											u->pc++;
										}
										cycle++;
										u->pc++;
									}
								}
								break;
							default:
								assert(false);
						}
					} else {
						//wrong op
						cycle = -1;
					}
				} else {
					//brbs, brbc
					bool clear = ((inst & 0x0400) == 0x0400);//brbc = 1, brbs = 0
					if (((u->sram[SREG] >> bit) & 1) ^ clear) {//bit set != clear
						//jump
						cycle++;
						int8_t delta = (inst >> 3) & 0x7F;
						if ((delta & 0x40) == 0x40) {// < 0
							delta |= 0x80;
						}
						u->pc += delta;
					}
				}
			} else {
				//wrong op
				cycle = -1;
			}
		}
	} else {
		//wrong op
		cycle = -1;
	}
	return cycle;
}
