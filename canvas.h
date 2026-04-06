/* file: canvas.h */

#ifndef CANVAS_H
#define CANVAS_H

#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>

#include "common.h"
#include "types.h"
#include "palette.h"

typedef struct {
	Palette *palette;
	int x,y,w,h;
	int nframe;
	int transparent;
	byte *pixels;
	byte gridColor;
	bool gridShow;
	int pixelSize;
	int frame;
} Canvas;

Canvas *Canvas_New(
    Palette *palette,
    int x, int y, int w, int h,
    int nframe,
    int transparent,
    byte color,
    byte gridColor,
    bool gridShow,
    int pixelSize,
    int frame
);

void Canvas_Free(Canvas *canvas);

Canvas *Canvas_LoadCVS(char *filename, Palette *palette);

void Canvas_Draw(Canvas *canvas, SDL_Renderer *renderer, SDL_Rect viewport);

void Canvas_DrawPoint(Canvas *canvas, int x, int y, byte color);
int Canvas_ReadPoint(Canvas *canvas, int x, int y);
void Canvas_DrawLine(Canvas *canvas, int x0, int y0, int x1, int y1, byte color);

void Canvas_MouseToCell(Canvas *canvas, int mx, int my, int *cx, int *cy);

void Canvas_FloodFill(Canvas *canvas, int x, int y, byte new_color, byte old_color);

#endif


