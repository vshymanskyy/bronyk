#include <SDL/SDL.h>
#include "SDL/SDL_image.h"
#include "SDL/SDL_ttf.h"

#include <algorithm>

static bool keyState[SDLK_LAST] = {};

void SDL_Put(SDL_Surface *dst, Sint16 x, Sint16 y, char symbol, TTF_Font* font, const SDL_Color& col) {
	char buff[2] = { symbol, '\0' };
	if (SDL_Surface *message = TTF_RenderText_Blended(font, buff, col)) {
		int minx, maxx, miny, maxy, advance;
		TTF_GlyphMetrics(font, symbol, &minx, &maxx, &miny, &maxy, &advance);
		SDL_Rect offset = { x + (minx+maxx)/2, y + (miny+maxy)/2 };
		SDL_BlitSurface(message, NULL, dst, &offset);
		SDL_FreeSurface(message);
	}
}



struct Object {
	char symbol;
	unsigned color;
	char descr[16];

	SDL_Color Color() const {
		SDL_Color c = {
				color >> 16 & 0xFF,
				color >>  8 & 0xFF,
				color >>  0 & 0xFF
		};
		return c;
	}
};

Object objects[256];
Object** mMap;
int mWidth;
int mHeight;
int mSize=20;

void LoadMap(const char* fn) {
	if (FILE* f = fopen(fn, "r")) {
		char line[128];
		while (fgets (line, sizeof(line), f) != NULL) {
parse_header:
			if (strcasecmp(line, "[Objects]\n") == 0) {
				for (int i=0; i<256; i++) {
					objects[i].symbol = i;
					objects[i].color = 0xFFFFFF;
				}
				while (fgets (line, sizeof(line), f) != NULL) {
					if (line[0] == '[') goto parse_header;
					if (strlen(line) == 0) continue;

					unsigned char symbol;
					int color;

					sscanf(line, "%c %x", &symbol, &color);

					Object &obj = objects[symbol];
					obj.symbol = symbol;
					obj.color = color;
				}
			} else if (strcasecmp(line, "[Map]\n") == 0) {
				// Find map dimensions
				mWidth = 0;
				mHeight = 0;
				long map_start = ftell(f);
				while (fgets (line, sizeof(line), f) != NULL) {
					if (line[0] == '[') break;
					mWidth = std::max(mWidth, (int)strlen(line));
					mHeight++;
				}

				mWidth--; // for newline

				// Allocate map
				mMap = new Object*[mWidth*mHeight];
		    	for (int x=0; x<mWidth; x++) {
			    	for (int y=0; y<mHeight; y++) {
			    		mMap[y*mWidth+x] = &objects[32];
			    	}
		    	}

				// Read map
				fseek(f, map_start, SEEK_SET);
				int y = 0;
				while (fgets (line, sizeof(line), f) != NULL) {
					if (line[0] == '[') break;
					int len = strlen(line)-1;
					for (int x=0; x<len; x++) {
						mMap[y*mWidth+x] = &objects[*((uint8_t*)&line[x])];
					}
					y++;
				}
			}
		}
		fclose(f);
	}
}


int main(int argc, char** argv) {
	const char* fn = "default.map";
	if (argc > 1) fn = argv[1];

	LoadMap(fn);

    if (SDL_Init(SDL_INIT_EVERYTHING) >= 0) {
	    if (SDL_Surface* screen = SDL_SetVideoMode((mWidth+1)*mSize, (mHeight+1)*mSize, 32, SDL_HWSURFACE)) {
	    	if( TTF_Init() == -1 ) { return 1; }

	    	TTF_Font *font = TTF_OpenFont("font.ttf", mSize);
	    	//TTF_SetFontStyle(font);

	    	SDL_WM_SetCaption(fn, NULL);
			SDL_Event event;
			while(SDL_WaitEvent(&event)) {
				switch (event.type){
				case SDL_QUIT:
					goto stop_drive;
				case SDL_KEYDOWN: {
					keyState[event.key.keysym.sym] = true;
					break;
				}
				case SDL_KEYUP: {
					keyState[event.key.keysym.sym] = false;
					break;
				}
				default:
					break;
				}

				SDL_FillRect( SDL_GetVideoSurface(), NULL, 0 );

		    	for (int x=0; x<mWidth; x++) {
			    	for (int y=0; y<mHeight; y++) {
			    		if (Object* obj = mMap[y*mWidth+x]) {

			    			SDL_Put(screen, x*mSize, y*mSize, obj->symbol, font, obj->Color());
			    		}

			    	}
		    	}

				SDL_Flip(screen);
			}
			SDL_FreeSurface(screen);
	    }
stop_drive:
	    SDL_Quit();
    }

	return 0;

}
