/* file: target.h */

#ifndef TARGET_H
#define TARGET_H

#include <SDL2/SDL.h>
#include <stdbool.h>
#include <math.h>

#include "palette.h"
#include "canvas.h"
#include "mouse.h"

typedef struct {
    Palette *palette;
    Canvas *canvas;
    int cellX, cellY;
    int x, y;
    bool visible;
} Target;

Target *Target_New(Palette *palette, Canvas *canvas, int px, int py);
void Target_Free(Target *target);

void Target_Update(Target *target, Mouse *mouse);
void Target_Draw(Target *target, SDL_Renderer *renderer);

#endif


