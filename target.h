#ifndef TARGET_H
#define TARGET_H

#include <SDL2/SDL.h>

#include "palette.h"
#include "canvas.h"




typedef struct {
	Palette *palette;
	Canvas *canvas;
	int x,y;
} Target;

Target *Target_New(Palette *palette, Canvas *canvas, int x, int y);
void Target_Draw(Target *target, SDL_Renderer *renderer);
void Target_Update(Target *target, Mouse *mouse);

#endif