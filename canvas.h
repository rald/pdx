#ifndef CANVAS_H
#define CANVAS_H

#include <SDL2/SDL.h>
#include <stdbool.h>

#include "common.h"
#include "types.h"

typedef struct {
	size_t w,h;
	size_t nframe;
	ssize_t transparent;
	byte *pixels;
	
	int x,y;
	byte gridColor;
	bool gridShow;
	size_t pixelSize;
	size_t frame;
	
	SDL_Color *palette;
	size_t npalette;
} Canvas;

Canvas *Canvas_New(
	int x, int y, 
	size_t w, size_t h, 
	size_t nframe, 
	ssize_t transparent, 
	byte color, 
	byte gridColor, 
	bool gridShow,  
	int pixelSize, 
	size_t frame,
	SDL_Color *palette,
	size_t npalette
);

void Canvas_Draw(Canvas *canvas, SDL_Renderer *renderer);
void Canvas_DrawPoint(Canvas *canvas, int x, int y, byte color);
ssize_t Canvas_ReadPoint(Canvas *canvas, int x, int y);

#endif
