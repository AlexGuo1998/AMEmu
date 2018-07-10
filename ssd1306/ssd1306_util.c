#include "ssd1306_api.h"

#include <stdlib.h>
#include <string.h>//memset
#include <assert.h>
#include <SDL.h>

SSD1306_HANDLE ssd1306_new(const char *wndname) {
	SSD1306_HANDLE h = (SSD1306_HANDLE)malloc(sizeof(SSD1606_STRU));
	assert(h);//TODO
	if (h == NULL) {
		return NULL;
	}
	if (SDL_InitSubSystem(SDL_INIT_VIDEO) != 0) {
		free(h);
		return NULL;
	}

	{
		SDL_Surface *img;
		img = SDL_LoadBMP(RESOURCE_FILE);
		assert(img);
		SDL_GetClipRect(img, &h->defrect);
		h->defrect.w /= 2;

		h->win = SDL_CreateWindow(wndname, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, h->defrect.w * 128, h->defrect.h * 64, SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI);
		h->ren = SDL_CreateRenderer(h->win, -1, SDL_RENDERER_ACCELERATED);
		h->tex = SDL_CreateTextureFromSurface(h->ren, img);
		SDL_FreeSurface(img);
	}
	//reset data
	{
		h->datachanged = true;
		h->remapchanged = true;
		h->isdatamode = false;

		h->contrast = 0x7F;
		h->entireon = false;
		h->inverse = false;
		h->displayon = false;

		h->scrolling = false;//TODO

		h->memaddrmode = 0b10;
		h->addrcolstart = 0;
		h->addrcolend = 127;
		h->addrpagestart = 0;
		h->addrpageend = 7;
		h->addrcolnow = 0;
		h->addrpagenow = 0;

		h->dispstartline = 0;
		h->segmentremap = false;
		h->muxratio = 64;
		h->comscandirection = false;
		h->displayoffset = 0;
		h->compinconf = true;
		h->comlrremap = false;

		h->clkdivratio = 0b0000;
		h->clkfreq = 0b1000;
		h->prechg1 = 0b0010;
		h->prechg2 = 0b0010;
		h->vcomhdeselect = 0b010;

		h->fadeblinkmode = 0b00;
		h->fadeblinktime = 0;//TODO
		h->zoomin = false;

		for (uint16_t i = 0; i < 8 * 128; i += 2) {
			((uint8_t *)&h->screenbuffer[0][0])[i] = 0x55;
			((uint8_t *)&h->screenbuffer[0][0])[i + 1] = 0xAA;
		}
		h->_cmdpart = 0;
		
		h->running = true;
	}
	return h;
}

void ssd1306_destroy(SSD1306_HANDLE h) {
	SDL_DestroyTexture(h->tex);
	SDL_DestroyRenderer(h->ren);
	SDL_DestroyWindow(h->win);
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
	free(h);
}

inline void ssd1306_dcselect(SSD1306_HANDLE h, bool isdata);

static const uint16_t scrollspeeddivlist[8] = {
	5, 64, 128, 256, 3, 4, 25, 2
};

static const uint16_t refreshfreqlist[16] = {
	0, //TODO...
};

void ssd1306_clock(SSD1306_HANDLE h) {
	//TODO
	//scrol-> movedata, datachanged = true
	//else return
}

void ssd1306_refresh(SSD1306_HANDLE h) {
	bool dorender = false;
	if (h->remapchanged) {
		dorender = true;
		h->remapchanged = false;
		//getremap
		if (!h->displayon) {
			//disp not on, black
			memset(h->_colmap, -1, 64);
		} else if (h->entireon) {
			//disp all on, white
			memset(h->_colmap, -2, 64);
		} else {
			for (uint8_t i = 0; i < 64; i++) {
				//TODO
				h->_colmap[i] = i;
			}
			if (h->zoomin) {
				for (uint8_t i = 0; i < 64; i++) {
					h->_colmap[i] >>= 1;
				}
			} else {
				if (!h->compinconf) {
					int8_t buf[32];
					memcpy(buf, h->_colmap, 32 * sizeof(h->_colmap[0]));
					//1=32, 3=33, 5=34, ..., 59=61, 61=62, 63=63
					for (uint8_t i = 32; i < 64; i++) {
						h->_colmap[((i & 0x1F) << 1) + 1] = h->_colmap[i];
					}
					//0=0, 2=1, 4=2
					for (uint8_t i = 0; i < 32; i++) {
						h->_colmap[i << 1] = buf[i];
					}
				}
				if (h->comlrremap) {
					uint8_t buf;
					for (uint8_t i = 0; i < 64; i += 2) {
						//swap 0 <-> 1, 2 <-> 3, ...
						buf = h->_colmap[i];
						h->_colmap[i] = h->_colmap[i + 1];
						h->_colmap[i + 1] = buf;
					}
				}
			}
		}
	}
	if (h->datachanged) {
		h->datachanged = false;
		dorender = true;
	}
	if (dorender) {
		SDL_Rect src = h->defrect, dst = h->defrect;
		SDL_RenderClear(h->ren);
		int srcxlist[2];
		if (!h->inverse) {
			srcxlist[0] = 0;//dark
			srcxlist[1] = src.w;//light
		} else {
			srcxlist[0] = src.w;//dark->light
			srcxlist[1] = 0;//light->dark
		}
		for (uint8_t y = 0; y < 64; y++) {
			dst.y = y * dst.h;
			int8_t buffery = h->_colmap[y];
			if (buffery >= 0) {
				//normal
				uint8_t colpage = h->_colmap[y] >> 3;
				uint8_t colbitmsk = 1 << (h->_colmap[y] & 0x07);
				dst.x = 0;
				for (uint8_t x = 0; x < 128; x++) {
					src.x = (h->screenbuffer[colpage][x] & colbitmsk) ? srcxlist[1] : srcxlist[0];
					SDL_RenderCopy(h->ren, h->tex, &src, &dst);
					dst.x += dst.w;
				}
			} else {
				//colmap < 0
				//-1 -> black, -2 -> white
				src.x = (buffery & 1) ? 0 : src.w;
				dst.x = 0;
				for (uint8_t x = 0; x < 128; x++) {
					SDL_RenderCopy(h->ren, h->tex, &src, &dst);
					dst.x += dst.w;
				}
			}
		}
		SDL_RenderPresent(h->ren);
	}
}
