#define _CRT_SECURE_NO_WARNINGS
#define SDL_MAIN_HANDLED

#include <stdio.h>
#include <string.h>
#include <assert.h>
#ifdef _WIN32
#include <Windows.h>
#endif // _WIN32

#include "ssd1306_api.h"
#include <SDL.h>

int SDLCALL thddatafetch(void *d) {
	SSD1306_HANDLE h = (SSD1306_HANDLE)d;
	char buf[128];
	int data;
	while (h->running) {
		fgets(buf, 128, stdin);
		if (sscanf(buf, "d%i", &data) == 1) {
			//data
			fprintf(stderr, "Data: 0x%02X\n", data & 0xFF);
			ssd1306_dcselect(h, true);
			ssd1306_spishiftin(h, data & 0xFF);
		} else if (sscanf(buf, "c%i", &data) == 1) {
			//command
			fprintf(stderr, "Cmd:  0x%02X\n", data & 0xFF);
			ssd1306_dcselect(h, false);
			ssd1306_spishiftin(h, data & 0xFF);
		} else if (buf[0] == 'n') {
			//next clock
			ssd1306_clock(h);
		} else if (strncmp(buf, "end", 3) == 0) {
			//end
			h->running = false;
		} else {
			fprintf(stderr, "Wrong command: %s", buf);
		}
	}
	return 0;
}

int main(int argc, char **argv) {
#ifdef _WIN32
	//SetProcessDPIAware();
#endif // _WIN32
	SSD1306_HANDLE h = ssd1306_new("SSD1306 Test");
	assert(h);
	SDL_Thread *thd = SDL_CreateThread(thddatafetch, "data fetch", h);
	bool waitthread = true;

	while (h->running) {
		SDL_Event e;
		while (SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT) {
				h->running = false;
				waitthread = false;
			}
			if (e.type == SDL_KEYDOWN) {
				//running = false;
			}
			if (e.type == SDL_MOUSEBUTTONDOWN) {
				//running = false;
			}
		}
		ssd1306_refresh(h);
		SDL_Delay(10);
	}

	if (waitthread) {
		SDL_WaitThread(thd, NULL);
	} else {
		SDL_DetachThread(thd);
	}
	ssd1306_destroy(h);
	return 0;
}
