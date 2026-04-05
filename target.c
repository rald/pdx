#include <stdlib.h>
#include "target.h"

static int Target_Step(Canvas *canvas) {
    return canvas->gridShow ? canvas->pixelSize + 1 : canvas->pixelSize;
}

static int Target_ToPixel(Canvas *canvas, int screenCoord) {
    return (screenCoord - canvas->x) / Target_Step(canvas);
}

static int Target_ToCenterX(Canvas *canvas, int pixelIndex) {
    int step = Target_Step(canvas);
    return canvas->x + pixelIndex * step + canvas->pixelSize / 2;
}

static int Target_ToCenterY(Canvas *canvas, int pixelIndex) {
    int step = Target_Step(canvas);
    return canvas->y + pixelIndex * step + canvas->pixelSize / 2;
}

Target *Target_New(Palette *palette, Canvas *canvas, int px, int py) {
    Target *target = malloc(sizeof(*target));
    if(!target) return NULL;

    target->palette = palette;
    target->canvas = canvas;
    target->px = px;
    target->py = py;
    target->visible = true;

    return target;
}

void Target_Free(Target *target) {
    free(target);
}

void Target_SetPixel(Target *target, int px, int py) {
    if(!target || !target->canvas) return;

    if(px >= 0 && px < target->canvas->w && py >= 0 && py < target->canvas->h) {
        target->px = px;
        target->py = py;
        target->visible = true;
    } else {
        target->visible = false;
    }
}

void Target_RecheckVisible(Target *target) {
    if(!target || !target->canvas) return;

    target->visible =
        target->px >= 0 && target->px < target->canvas->w &&
        target->py >= 0 && target->py < target->canvas->h;
}

void Target_Update(Target *target, Mouse *mouse) {
    if(!target || !target->canvas || !mouse) return;

    if(mouse->state & SDL_BUTTON_RMASK) {
        int px = Target_ToPixel(target->canvas, mouse->x);
        int py = Target_ToPixel(target->canvas, mouse->y);
        Target_SetPixel(target, px, py);
    }
}

void Target_Draw(Target *target, SDL_Renderer *renderer) {
    if(!target || !target->canvas || !target->visible) return;

    int x = Target_ToCenterX(target->canvas, target->px);
    int y = Target_ToCenterY(target->canvas, target->py);

    int k = target->canvas->frame * target->canvas->w * target->canvas->h
          + target->py * target->canvas->w
          + target->px;

    byte p = target->canvas->pixels[k];
    SDL_Color c = target->palette->colors[p];

    SDL_SetRenderDrawColor(renderer, 255 - c.r, 255 - c.g, 255 - c.b, 255);

    SDL_RenderDrawLine(renderer, x - 4, y, x + 4, y);
    SDL_RenderDrawLine(renderer, x, y - 4, x, y + 4);
}


