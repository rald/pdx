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
    if(mouse->state & SDL_BUTTON_RMASK) {
        int localX = mouse->x - target->canvas->x;
        int localY = mouse->y - target->canvas->y;
        int step = target->canvas->gridShow ? target->canvas->pixelSize + 1 : target->canvas->pixelSize;

        if(step <= 0) return;

        target->x = target->canvas->x + (localX / step) * step + (target->canvas->pixelSize / 2);
        target->y = target->canvas->y + (localY / step) * step + (target->canvas->pixelSize / 2);
    }
}

void Target_Draw(Target *target, SDL_Renderer *renderer) {
    int i, j;
    Uint32 pixel;
    Uint8 r, g, b, a;

    int w = 4 - -4 + 1, h = 4 - -4 + 1;
    SDL_Rect area = { target->x - 4, target->y - 4, w, h };

    SDL_Window *window = SDL_RenderGetWindow(renderer);
    SDL_Surface *surface = SDL_GetWindowSurface(window);
    Uint32 format = SDL_GetWindowPixelFormat(window);
    int pitch = w * SDL_BYTESPERPIXEL(format);
    Uint32 *pixels = malloc(pitch * h);

    if(!pixels) return;

	SDL_Rect clip = {0,0,640-16,480-32-16};
	
	SDL_RenderSetClipRect(renderer,&clip);

    if (SDL_RenderReadPixels(renderer, &area, format, pixels, pitch) == 0) {
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

        for(i = -4; i <= 4; i++) {
            int sx = i + 4;
            int sy = 4;
            pixel = pixels[sy * w + sx];
            SDL_GetRGBA(pixel, surface->format, &r, &g, &b, &a);
            SDL_SetRenderDrawColor(renderer, 255 - r, 255 - g, 255 - b, a);
            SDL_RenderDrawPoint(renderer, target->x + i, target->y);
        }

        for(j = -4; j <= 4; j++) {
            int sx = 4;
            int sy = j + 4;
            pixel = pixels[sy * w + sx];
            SDL_GetRGBA(pixel, surface->format, &r, &g, &b, &a);
            SDL_SetRenderDrawColor(renderer, 255 - r, 255 - g, 255 - b, a);
            SDL_RenderDrawPoint(renderer, target->x, target->y + j);
        }
    }

	SDL_RenderSetClipRect(renderer,NULL);

    free(pixels);
}

