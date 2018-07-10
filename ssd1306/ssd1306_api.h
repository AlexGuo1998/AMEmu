#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <SDL_video.h>
#include <SDL_render.h>

#include "ssd1306_config.h"

typedef struct SSD1306_STRU_t {
	uint8_t screenbuffer[8][128];
	bool running;
	bool datachanged;
	bool remapchanged;
	bool isdatamode;
	uint16_t clockdura;

	//fundermental
	uint8_t contrast;	// = 0x7F
	bool entireon;		// = false
	bool inverse;		// = false
	bool displayon;		// = false

	//scrolling
	bool scrolling;			// = false
	bool scrollxspeed;		// 0=right 1=left
	uint8_t scrollyspeed;
	uint8_t scrollpstart;	//start page
	uint8_t scrollpend;		//end page
	uint8_t scrollspeeddiv;	//speed division, in scrollspeeddivlist[]
	uint8_t scrollstarty;	//start y line
	uint8_t scrollheight;	//scroll area height

	//addressing
	uint8_t memaddrmode;	// = 0b10
	uint8_t addrcolstart;	// = 0
	uint8_t addrcolend;		// = 127
	uint8_t addrpagestart;	// = 0
	uint8_t addrpageend;	// = 7
	uint8_t addrcolnow;		// 0 ~ 127
	uint8_t addrpagenow;	// 0 ~ 7

	//display
	uint8_t dispstartline;	// = 0
	bool segmentremap;		// = false
	uint8_t muxratio;		// = 64
	bool comscandirection;	// = false
	uint8_t displayoffset;	// = 0
	bool compinconf;		// = true
	//false: ram 0-64 -> scr 0,2,4,...,62,1,3,5,...,63
	//true:  ram 0-64 -> scr 0,1,2,...,63
	bool comlrremap;		// = false
	//true: swap 0 <-> 1, 2 <-> 2, ... , 62 <-> 63

	//timing / driving
	uint8_t clkdivratio : 4;// = 0b0000
	uint8_t clkfreq : 4;// = 0b1000
	uint8_t prechg1 : 4;// = 0b0010
	uint8_t prechg2 : 4;// = 0b0010
	uint8_t vcomhdeselect;// = 0b010

	 //advance graphic
	uint8_t fadeblinkmode;// = 0b00
	uint8_t fadeblinktime;// (t + 1) << 3 frames
	bool zoomin;// = false
#ifdef CHARGE_PUMP_AVALIABLE
	bool pumpenabled;// = false
#endif

	//internal
	int8_t _colmap[64];
	uint8_t _cmdpart;

	//SDL
	SDL_Window *win;
	SDL_Renderer *ren;
	SDL_Texture *tex;
	SDL_Rect defrect;
} SSD1606_STRU, *SSD1306_HANDLE;

SSD1306_HANDLE ssd1306_new(const char *name);
void ssd1306_destroy(SSD1306_HANDLE h);

inline void ssd1306_dcselect(SSD1306_HANDLE h, bool isdata) {
	h->isdatamode = isdata;
}
void ssd1306_spishiftin(SSD1306_HANDLE h, uint8_t data);
void ssd1306_clock(SSD1306_HANDLE h);
void ssd1306_refresh(SSD1306_HANDLE h);

//should be implemented by user
extern void DebugPrint(const char *msg);
