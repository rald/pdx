#ifndef PALETTE_H
#define PALETTE_H

#include "common.h"
#include "types.h"
#include "utils.h"
#include "mouse.h"

typedef struct {
	int x,y,w,h;
	SDL_Color *colors;
	size_t ncolors;
	byte currentColor;
	size_t boxSize;
} Palette;

Palette *Palette_New(SDL_Color *colors, size_t ncolors, int x, int y, int w, int h, byte currentColor, size_t size);

void Palette_EventHandle(Palette *palette,SDL_Event event);
void Palette_Update(Palette *palette, Mouse *mouse);
void Palette_Draw(Palette *palette, SDL_Renderer *renderer, Mouse *mouse);

#endif