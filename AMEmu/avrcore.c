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

#define GETC() (u->reg[SREG] & (1 << SREG_C))
#define GETZ() (u->reg[SREG] & (1 << SREG_Z))
#define GETN() (u->reg[SREG] & (1 << SREG_N))
#define GETV() (u->reg[SREG] & (1 << SREG_V))
#define GETS() (u->reg[SREG] & (1 << SREG_S))
#define GETH() (u->reg[SREG] & (1 << SREG_H))
#define GETT() (u->reg[SREG] & (1 << SREG_T))
#define GETI() (u->reg[SREG] & (1 << SREG_I))

#define SETC(x) (u->reg[SREG] = (u->reg[SREG] & ~(1 << SREG_C)) | ((x) ? 1 << SREG_C : 0))
#define SETZ(x) (u->reg[SREG] = (u->reg[SREG] & ~(1 << SREG_Z)) | ((x) ? 1 << SREG_Z : 0))
#define SETN(x) (u->reg[SREG] = (u->reg[SREG] & ~(1 << SREG_N)) | ((x) ? 1 << SREG_N : 0))
#define SETV(x) (u->reg[SREG] = (u->reg[SREG] & ~(1 << SREG_V)) | ((x) ? 1 << SREG_V : 0))
#define SETS(x) (u->reg[SREG] = (u->reg[SREG] & ~(1 << SREG_S)) | ((x) ? 1 << SREG_S : 0))
#define SETH(x) (u->reg[SREG] = (u->reg[SREG] & ~(1 << SREG_H)) | ((x) ? 1 << SREG_H : 0))
#define SETT(x) (u->reg[SREG] = (u->reg[SREG] & ~(1 << SREG_T)) | ((x) ? 1 << SREG_T : 0))
#define SETI(x) (u->reg[SREG] = (u->reg[SREG] & ~(1 << SREG_I)) | ((x) ? 1 << SREG_I : 0))

#pragma endregion

#define LOW(data) ((data) & 0xFF)
#define HIGH(data) (((data) >> 8) & 0xFF)


int avr_runstep(AVRMCU *u) {
	int cycle = 1;
	uint16_t inst = u->rom[u->pc];
	u->pc++;
	//process operation
	if (inst == 0) {
		//nop
		//break;
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
						int8_t d = (int8_t)u->reg[rd];
						uint8_t r = u->reg[rr];
						int32_t o = d * r;
						SETZ(o == 0);
						SETC(o & (1 << 15));
						u->reg[0] = LOW(o);
						u->reg[1] = HIGH(o);
					}
				} else {
					//fmul
					{
						uint8_t d = u->reg[rd];
						uint8_t r = u->reg[rr];
						uint32_t o = d * r;
						SETZ(o == 0);
						SETC(o & (1 << 15));
						o <<= 1;
						u->reg[0] = LOW(o);
						u->reg[1] = HIGH(o);
					}
				}
			} else {
				if ((inst & 0x0008) == 0x0000) {
					//fmuls
					{
						int8_t d = (int8_t)u->reg[rd];
						int8_t r = (int8_t)u->reg[rr];
						int32_t o = d * r;
						SETZ(o == 0);
						SETC(o & (1 << 15));
						o <<= 1;
						u->reg[0] = LOW(o);
						u->reg[1] = HIGH(o);
					}
				} else {
					//fmulsu
					{
						int8_t d = (int8_t)u->reg[rd];
						uint8_t r = u->reg[rr];
						int32_t o = d * r;
						SETZ(o == 0);
						SETC(o & (1 << 15));
						o <<= 1;
						u->reg[0] = LOW(o);
						u->reg[1] = HIGH(o);
					}
				}
			}
		} else if ((inst & 0x0300) == 0x0100) {
			//movw
			{
				uint8_t rd, rr;
				rd = (inst & 0x00F0) >> 3;
				rr = (inst & 0x000F) << 1;
				u->reg[rd] = u->reg[rr];
				u->reg[rd + 1] = u->reg[rr + 1];
			}
		} else if ((inst & 0x0300) == 0x0200) {
			//muls
			{
				uint8_t rd, rr;
				rd = ((inst & 0x00F0) >> 4) | 0x10;
				rr = (inst & 0x000F) | 0x10;

				int8_t d = (int8_t)u->reg[rd];
				int8_t r = (int8_t)u->reg[rr];
				int32_t o = d * r;
				SETZ(o == 0);
				SETC(o & (1 << 15));
				u->reg[0] = LOW(o);
				u->reg[1] = HIGH(o);
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
		d = u->reg[rd];
		r = u->reg[rr];
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
						u->reg[rd] = o;
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
						u->reg[rd] = o;
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
						u->reg[rd] = o;
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
						u->reg[rd] = o;
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
						u->reg[rd] = o;
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
						u->reg[rd] = o;
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
						u->reg[rd] = o;
					}
					break;
				case 0x0B:
					//mov
					{
						u->reg[rd] = r;
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
				u->reg[0] = LOW(o);
				u->reg[1] = HIGH(o);
			}
		}
	} else if (((inst & 0x8000) == 0x0000) || ((inst & 0xF000) == 0xE000)) {
		//TODO
		uint8_t rd, k;
		rd = ((inst >> 4) & 0x0F) | 0x10;
		k = (inst & 0x0F) | ((inst >> 4) & 0xF0);
		switch (inst >> 12) {
			case 0x3:
				//cpi
				break;
			case 0x4:
				//sbci
				break;
			case 0x5:
				//subi
				break;
			case 0x6:
				//ori
				break;
			case 0x7:
				//andi
				break;
			case 0xE:
				//ldi
				break;
			default:
				assert(false);
		}
	} else if (0) {

	}
	return cycle;
}

int8_t avr_runstep_1(AVRMCU *u) {
	int8_t cycle = 1;
	uint16_t inst = u->rom[u->pc];
	switch (inst >> 12) {
	case 0b0000:
		if (inst == 0) {
			//nop
			break;
		}
		if ((inst & 0x0C00) == 0) {

		} else {

		}

		break;
	case 0b0001:
		//cpse, cp, sub, adc
		break;
	case 0b0010:
		//and, xor, or, mov
		break;



	case 0b0011:
		//cpi
	case 0b0100:
		//sbci
	case 0b0101:
		//subi
	case 0b0110:
		//ori
	case 0b0111:
		//andi
	case 0b1110:
		//ldi(OK)
		{
			uint8_t reg = 0x10 | ((inst >> 4) & 0x0F);
			uint8_t data = ((inst >> 4) & 0xF0) | (inst & 0x0F);
			switch (inst >> 12) {
			case 0b0011:
				//cpi
				break;
			case 0b0100:
				//sbci
				break;
			case 0b0101:
				//subi
				break;
			case 0b0110:
				//ori
				break;
			case 0b0111:
				//andi
				break;
			case 0b1110:
				//ldi
				u->reg[reg] = data;
				break;
			default:
				assert(false);
			}
		}
		break;
	case 0b1000:
		break;
	case 0b1001:
		break;
	case 0b1010:
		break;
	case 0b1011:
		//in, out(OK)
		{
			uint8_t reg = (inst >> 4) & 0x1F;
			uint8_t io = (((inst >> 5) & 0x30) | (inst & 0x0F)) + 0x20;
			if (inst & 0x0800) {
				//out
				u->reg[io] = u->reg[reg];
			} else {
				//in
				u->reg[reg] = u->reg[io];
			}
		}
		break;
	case 0b1100:
		//rjmp
		break;
	case 0b1101:
		//rcall
		break;
	case 0b1111:
		break;

	}
	u->pc++;
	return cycle;
}

