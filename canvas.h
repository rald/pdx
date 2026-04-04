#ifndef CANVAS_H
#define CANVAS_H

#include <SDL2/SDL.h>
#include <stdbool.h>

#include "common.h"
#include "types.h"
#include "palette.h"



typedef struct {
	Palette *palette;

	int w,h;
	int nframe;
	int transparent;
	byte *pixels;
	
	int x,y;
	byte gridColor;
	bool gridShow;
	int pixelSize;
	int frame;
	} Canvas;

Canvas *Canvas_New(
	Palette *palette,
	int x, int y, 
	int w, int h, 
	int nframe, 
	int transparent, 
	byte color, 
	byte gridColor, 
	bool gridShow,  
	int pixelSize, 
	int frame
);

void Canvas_EventHandle(Canvas *canvas,SDL_Event event);
void Canvas_Update(Canvas *canvas, Mouse *mouse);
void Canvas_Draw(Canvas *canvas, SDL_Renderer *renderer);
void Canvas_DrawPoint(Canvas *canvas, int x, int y, byte color);
ssize_t Canvas_ReadPoint(Canvas *canvas, int x, int y);

#endif
