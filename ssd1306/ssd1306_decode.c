#include "ssd1306_api.h"

#include <assert.h>

#define MEMORY_ADDRESSING_MODE_HORIZONTAL	0b00
#define MEMORY_ADDRESSING_MODE_VERTICAL		0b01
#define MEMORY_ADDRESSING_MODE_PAGE			0b10

#define CMDPART_SET_CONTRAST_CONTROL								1
#define CMDPART_CONTINUOUS_HORIZONTAL_SCROLL_SETUP					2 //to 7
#define CMDPART_CONTINUOUS_VERTICAL_AND_HORIZONTAL_SCROLL_SETUP 	8 //to 12
#define CMDPART_SET_VERTICAL_SCROLL_AREA							13 //to 14
#define CMDPART_SET_MEMORY_ADDRESSING_MODE							15
#define CMDPART_SET_COLUMN_RANGE									16 //to 17
#define CMDPART_SET_PAGE_RANGE										18 //to 19
#define CMDPART_SET_MULTIPLEX_RATIO									20
#define CMDPART_SET_DISPLAY_OFFSET									21
#define CMDPART_SET_COM_PINS_HARDWARE_CONFIGURATION					22
#define CMDPART_SET_DISPLAY_CLOCK_DIVIDE_RATIO_OSCILLATOR_FREQUENCY	23
#define CMDPART_SET_PRE_CHARGE_PERIOD								24
#define CMDPART_SET_VCOMH_DESELECT_LEVEL							25
#define CMDPART_SET_FADEOUT_AND_BLINKING							26
#define CMDPART_SET_ZOOM_IN											27
#ifdef CHARGE_PUMP_AVALIABLE
#define CMDPART_CHARGE_PUMP_SETTING									28
#endif



#define CMDERR_INVALID_COMMAND									1
#define CMDERR_INVALID_HVSCROLL_SETTING							2
#define CMDERR_INVALID_MEMORY_ADDRESSING_MODE					3
#define CMDERR_INVALID_MUX_RATIO								4
#define CMDERR_WRONG_COM_PINS_HARDWARE_CONFIGURATION_SEQUENCE	5
#define CMDERR_WRONG_VCOMH_SETTING_SEQUENCE						6
#define CMDERR_INVALID_VCOMH_SETTING							7
#define CMDERR_INVALID_FADEOUT_BLINK_SETTING					8
#define CMDERR_WRONG_ZOOMIN_SEQUENCE							9
#ifdef CHARGE_PUMP_AVALIABLE
#define CMDERR_WRONG_PUMP_SEQUENCE								10
#endif

static inline int docmd(SSD1306_HANDLE h, uint8_t d) {
	if (h->_cmdpart == 0) {
		if (d == 0b10000001) {
			//Set Contrast Control
			h->_cmdpart = CMDPART_SET_CONTRAST_CONTROL;
		} else if ((d & 0b11111110) == 0b10100100) {
			//Entire Display ON
			h->entireon = d & 0b1;
		} else if ((d & 0b11111110) == 0b10100110) {
			//Set Normal/Inverse Display
			h->inverse = d & 0b1;
		} else if ((d & 0b11111110) == 0b10101110) {
			//Set Display ON/OFF
			h->displayon = d & 0b1;
		} else if ((d & 0b11111110) == 0b00100110) {
			//Continuous Horzonatal Scroll Setup
			h->scrollxspeed = d & 0b00000001;
			h->scrollyspeed = 0;
			h->_cmdpart = CMDPART_CONTINUOUS_HORIZONTAL_SCROLL_SETUP;
		} else if ((d & 0b11111100) == 0b00101000) {
			//Continuous Vertical and Horzonatal Scroll Setup
			if (!((d & 0b00000001) ^ ((d & 0b00000010) >> 1))) {
				return CMDERR_INVALID_HVSCROLL_SETTING;
			}
			h->scrollxspeed = (d & 0b00000001) ^ 0b00000001;
			h->_cmdpart = CMDPART_CONTINUOUS_VERTICAL_AND_HORIZONTAL_SCROLL_SETUP;
		} else if (d == 0b00101110) {
			//Deactive scroll
			h->scrolling = false;
		} else if (d == 0b00101111) {
			//Active scroll
			h->scrolling = true;
		} else if (d == 0b10100011) {
			//Set Vertical Scroll Area
			h->_cmdpart = CMDPART_SET_VERTICAL_SCROLL_AREA;
		} else if ((d & 0b11110000) == 0b00000000) {
			//Set Lower Column Start Address for Page Addressing Mode
			//NOTE: Also work on Horzonal Mode & Vertical Mode
			h->addrcolnow = (h->addrcolnow & 0xF0) | (d & 0x0F);
		} else if ((d & 0b11110000) == 0b00010000) {
			//Set Higher Column Start Address for Page Addressing Mode
			//addr >= 128 -> wraparound
			h->addrcolnow = (h->addrcolnow & 0x0F) | ((d & 0x07) << 4);
		} else if (d == 0b00100000) {
			//Set Memory Addressing Mode
			h->_cmdpart = CMDPART_SET_MEMORY_ADDRESSING_MODE;
		} else if (d == 0b00100001) {
			//Set Column Address (range)
			h->_cmdpart = CMDPART_SET_COLUMN_RANGE;
		} else if (d == 0b00100010) {
			//Set Page Address (range)
			h->_cmdpart = CMDPART_SET_PAGE_RANGE;
		} else if ((d & 0b11111000) == 0b10110000) {
			//Set Page Start Address for Page Addressing Mode
			//TODO page only?
			h->addrpagenow = d & 0b00000111;
		} else if ((d & 0b11000000) == 0b01000000) {
			//Set Display Start Line
			h->dispstartline = d & 0b00111111;
		} else if ((d & 0b11111110) == 0b10100000) {
			//Set Segment Re-map
			h->segmentremap = d & 0b00000001;
		} else if (d == 0b10101000) {
			//Set Multiplex Ratio
			h->_cmdpart = CMDPART_SET_MULTIPLEX_RATIO;
		} else if ((d & 0b11110111) == 0b11000000) {
			//Set COM Output Scan Direction
			h->comscandirection = d >> 3;
		} else if (d == 0b11010011) {
			//Set Display Offset
			h->_cmdpart = CMDPART_SET_DISPLAY_OFFSET;
		} else if (d == 0b11011010) {
			//Set COM Pins Hardware Configuration
			h->_cmdpart = CMDPART_SET_COM_PINS_HARDWARE_CONFIGURATION;
		} else if (d == 0b11010101) {
			//Set Display Clock Divide Ratio / Oscillator Frequency
			h->_cmdpart = CMDPART_SET_DISPLAY_CLOCK_DIVIDE_RATIO_OSCILLATOR_FREQUENCY;
		} else if (d == 0b11011001) {
			//Set Pre-charge Period
			h->_cmdpart = CMDPART_SET_PRE_CHARGE_PERIOD;
		} else if (d == 0b11011011) {
			//Set V_COMH Deselect Level
			h->_cmdpart = CMDPART_SET_VCOMH_DESELECT_LEVEL;
		} else if (d == 0b00100011) {
			//Set Fadeout and Blinking
			h->_cmdpart = CMDPART_SET_FADEOUT_AND_BLINKING;
		} else if (d == 0b11010110) {
			//Set Zoom In
			h->_cmdpart = CMDPART_SET_ZOOM_IN;
		} else if (d == 0b11100011) {
			//NOP
#ifdef CHARGE_PUMP_AVALIABLE
		} else if (d == 0b10001101) {
			//Set V_COMH Deselect Level
			h->_cmdpart = CMDPART_CHARGE_PUMP_SETTING;
#endif
		} else {
			//CMD Error
			return CMDERR_INVALID_COMMAND;
		}
	} else if (h->_cmdpart == CMDPART_SET_CONTRAST_CONTROL) {
		h->contrast = d;
		h->_cmdpart = 0;
	} else if (h->_cmdpart >= CMDPART_CONTINUOUS_HORIZONTAL_SCROLL_SETUP &&
		h->_cmdpart < CMDPART_CONTINUOUS_HORIZONTAL_SCROLL_SETUP + 6) {
		switch (h->_cmdpart - CMDPART_CONTINUOUS_HORIZONTAL_SCROLL_SETUP) {
		case 0: case 1: case 2: case 3:
			//TODO
			break;
		case 4:
			break;
		case 5:
			h->_cmdpart = 0;
			if (d != 0b11111111) {
				//TODO
			}
			break;
		default:;
			assert(false);
		}
		h->_cmdpart++;
	} else if (h->_cmdpart >= CMDPART_CONTINUOUS_VERTICAL_AND_HORIZONTAL_SCROLL_SETUP &&
		h->_cmdpart < CMDPART_CONTINUOUS_VERTICAL_AND_HORIZONTAL_SCROLL_SETUP + 5) {
		switch (h->_cmdpart - CMDPART_CONTINUOUS_HORIZONTAL_SCROLL_SETUP) {
		case 0: case 1: case 2: case 3:
			//TODO
			break;
		case 4:
			h->_cmdpart = 0;
			//TODO
			break;
		default:;
			assert(false);
		}
		h->_cmdpart++;
	} else if (h->_cmdpart == CMDPART_SET_VERTICAL_SCROLL_AREA) {
		h->scrollstarty = d & 0b00111111;
		h->_cmdpart = CMDPART_SET_VERTICAL_SCROLL_AREA + 1;
	} else if (h->_cmdpart == CMDPART_SET_VERTICAL_SCROLL_AREA + 1) {
		//TODO: check valid
		d &= 0b01111111;
		h->scrollheight = d;
		h->_cmdpart = 0;
	} else if (h->_cmdpart == CMDPART_SET_MEMORY_ADDRESSING_MODE) {
		d &= 0b00000011;
		h->_cmdpart = 0;
		if (d == 0b11) {
			return CMDERR_INVALID_MEMORY_ADDRESSING_MODE;
		}
		h->memaddrmode = d;
	} else if (h->_cmdpart == CMDPART_SET_COLUMN_RANGE) {
		h->addrcolstart = d & 0b01111111;
		h->_cmdpart = CMDPART_SET_COLUMN_RANGE + 1;
	} else if (h->_cmdpart == CMDPART_SET_COLUMN_RANGE + 1) {
		h->addrcolend = d & 0b01111111;
		//don't check for end < start
		h->addrcolnow = h->addrcolstart;
		h->_cmdpart = 0;
	} else if (h->_cmdpart == CMDPART_SET_PAGE_RANGE) {
		h->addrpagestart = d & 0b00000111;
		h->_cmdpart = CMDPART_SET_PAGE_RANGE + 1;
	} else if (h->_cmdpart == CMDPART_SET_PAGE_RANGE + 1) {
		h->addrpageend = d & 0b00000111;
		h->addrpagenow = h->addrpagestart;
		h->_cmdpart = 0;
	} else if (h->_cmdpart == CMDPART_SET_MULTIPLEX_RATIO) {
		d &= 0b00111111;
		h->_cmdpart = 0;
		if (d <= 14) {
			return CMDERR_INVALID_MUX_RATIO;
		}
		h->muxratio = d + 1;
	} else if (h->_cmdpart == CMDPART_SET_DISPLAY_OFFSET) {
		h->displayoffset = d & 0b00111111;
		h->_cmdpart = 0;
	} else if (h->_cmdpart == CMDPART_SET_COM_PINS_HARDWARE_CONFIGURATION) {
		h->_cmdpart = 0;
		if ((d & 0b11001111) != 0b00000010) {
			return CMDERR_WRONG_COM_PINS_HARDWARE_CONFIGURATION_SEQUENCE;
		}
		h->compinconf = (d >> 4) & 0b1;
		h->comlrremap = (d >> 5) & 0b1;
	} else if (h->_cmdpart == CMDPART_SET_DISPLAY_CLOCK_DIVIDE_RATIO_OSCILLATOR_FREQUENCY) {
		h->clkdivratio = d & 0b00001111;
		h->clkfreq = d >> 4;
		h->_cmdpart = 0;
	} else if (h->_cmdpart == CMDPART_SET_PRE_CHARGE_PERIOD) {
		h->prechg1 = d & 0b00001111;
		h->prechg2 = d >> 4;
		h->_cmdpart = 0;
	} else if (h->_cmdpart == CMDPART_SET_VCOMH_DESELECT_LEVEL) {
		h->_cmdpart = 0;
		if ((d & 0b10001111) != 0b00000000) {
			return CMDERR_WRONG_VCOMH_SETTING_SEQUENCE;
		}
		switch (d & 0b01110000) {
		case 0b00000000: case 0b00100000: case 0b00110000:
			h->vcomhdeselect = (d >> 4) & 0b0111;
			break;
		default:
			return CMDERR_INVALID_VCOMH_SETTING;
		}
	} else if (h->_cmdpart == CMDPART_SET_FADEOUT_AND_BLINKING) {
		h->_cmdpart = 0;
		if ((d & 0b00110000) == 0b00010000) {
			return CMDERR_INVALID_FADEOUT_BLINK_SETTING;
		}
		h->fadeblinkmode = (d >> 4) & 0b11;
		h->fadeblinktime = d & 0b1111;
	} else if (h->_cmdpart == CMDPART_SET_ZOOM_IN) {
		h->_cmdpart = 0;
		if ((d & 0b11111110) != 0b00000000) {
			return CMDERR_WRONG_ZOOMIN_SEQUENCE;
		}
		h->zoomin = d & 0b1;
#ifdef CHARGE_PUMP_AVALIABLE
	} else if (h->_cmdpart == CMDPART_CHARGE_PUMP_SETTING) {
		h->_cmdpart = 0;
		if ((d & 0b00111011) != 0b00010000) {
			return CMDERR_WRONG_PUMP_SEQUENCE;
		}
		h->pumpenabled = (d & 0b00000100) ? true : false;
#endif
	} else {
		assert(false);
	}
	return 0;
}

static inline void dodata(SSD1306_HANDLE h, uint8_t d) {
	assert(h->addrpagenow < 8);
	assert(h->addrcolnow < 128);
	uint8_t paintcolnow;
	if (h->segmentremap) {
		paintcolnow = h->addrcolnow;
	} else {
		paintcolnow = 127 - h->addrcolnow;
	}
#ifdef REDUCE_REFRESHING
	if (h->screenbuffer[h->addrpagenow][paintcolnow] != d) {
		h->screenbuffer[h->addrpagenow][paintcolnow] = d;
		h->datachanged = true;
	}
#else // !REDUCE_REFRESHING
	h->screenbuffer[h->addrpagenow][paintcolnow] = d;
	h->datachanged = true;
#endif // REDUCE_REFRESHING
	if (h->memaddrmode == MEMORY_ADDRESSING_MODE_HORIZONTAL) {
		h->addrcolnow += 1;
		if (h->addrcolnow > h->addrcolend) {
			h->addrcolnow = h->addrcolstart;
			h->addrpagenow += 1;
		}
		if (h->addrpagenow > h->addrpageend) {
			h->addrpagenow = h->addrpagestart;
		}
	} else if (h->memaddrmode == MEMORY_ADDRESSING_MODE_VERTICAL) {
		h->addrpagenow += 1;
		if (h->addrpagenow > h->addrpageend) {
			h->addrpagenow = h->addrpagestart;
			h->addrcolnow += 1;
		}
		if (h->addrcolnow > h->addrcolend) {
			h->addrcolnow = h->addrcolstart;
		}
	} else if (h->memaddrmode == MEMORY_ADDRESSING_MODE_PAGE) {
		h->addrcolnow += 1;
		if (h->addrcolnow > h->addrcolend) {
			h->addrcolnow = h->addrcolstart;
		}
		if (h->addrpagenow > h->addrpageend) {
			h->addrpagenow = h->addrpagestart;
		}
	} else {
		//memaddrmode error
		assert(false);
	}
}

void ssd1306_spishiftin(SSD1306_HANDLE h, uint8_t d) {
	if (h->isdatamode) {
		dodata(h, d);
	} else {
		int ret = docmd(h, d);
		if (ret != 0) {
			//TODO report error
		}
		h->remapchanged = true;//TODO
	}
}
