/* file: target.c */

#include <stdlib.h>
#include "target.h"

Target *Target_New(Palette *palette, Canvas *canvas, int x, int y) {
    Target *target = malloc(sizeof(*target));
    if(!target) return NULL;

    target->palette = palette;
    target->canvas = canvas;
    target->x = x;
    target->y = y;
    target->visible = true;
    return target;
}

void Target_Free(Target *target) {
    free(target);
}

void Target_Update(Target *target, Mouse *mouse) {
    if(!(mouse->state & SDL_BUTTON_RMASK)) return;

    int step = target->canvas->gridShow ? target->canvas->pixelSize + 1 : target->canvas->pixelSize;
    if(step <= 0) return;

    int localX = mouse->x - target->canvas->x;
    int localY = mouse->y - target->canvas->y;

    int cellX = localX / step;
    int cellY = localY / step;

    if(localX < 0) cellX--;
    if(localY < 0) cellY--;

    target->x = target->canvas->x + cellX * step + (target->canvas->pixelSize / 2);
    target->y = target->canvas->y + cellY * step + (target->canvas->pixelSize / 2);
}

void Target_Draw(Target *target, SDL_Renderer *renderer) {
    int s = 4;
    int x = target->x;
    int y = target->y;

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawLine(renderer, x - s, y, x + s, y);
    SDL_RenderDrawLine(renderer, x, y - s, x, y + s);

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderDrawPoint(renderer, x - 1, y);
    SDL_RenderDrawPoint(renderer, x + 1, y);
    SDL_RenderDrawPoint(renderer, x, y - 1);
    SDL_RenderDrawPoint(renderer, x, y + 1);
}


