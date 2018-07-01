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

inline static bool MemWrite(AVRMCU *u, uint16_t addr, uint8_t data) {
	if (addr >= 0x20 && addr <= 0xFF) {
		//IO
		//TODO IO callback
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
	if (addr >= 0x20 && addr <= 0xFF) {
		//IO
		//TODO IO callback
	} else {
		//direct
		if (!CheckSRAMAddr(addr)) {
			return false;
		}
		*data = u->sram[addr];
	}
	return true;
}

inline static bool PushPC(AVRMCU *u) {
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
				//TODO
			}
		} else {
			//ld(d), st(d)
			cycle++;
			uint8_t delta;//TODO read
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
						cycle++;
						//jump
						uint16_t delta;//TODO read relative value
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
	//TODO test PC wraparound?
	return cycle;
}
