#include "canvas.h"

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
		size_t npalette) {

	Canvas *canvas = malloc(sizeof(*canvas));
	if(canvas) {
		canvas->w = w;
		canvas->h = h;
		canvas->transparent = transparent;

		canvas->pixels = calloc(w * h * nframe,sizeof(*canvas->pixels));
		for(size_t i = 0; i < w * h * nframe; i++) {
			canvas->pixels[i]=color;
		}

		canvas->pixelSize = pixelSize;		
		canvas->x = x;
		canvas->y = y;
		canvas->frame = frame;
		canvas->gridColor = gridColor;
		canvas->gridShow = gridShow;
		
		canvas->palette = palette;
		canvas->npalette = npalette;
	}

	return canvas;
}

void Canvas_Draw(Canvas *canvas, SDL_Renderer *renderer) {
	SDL_Rect rect;

	SDL_Rect clip = { 0, 0, SCREEN_WIDTH - 16, SCREEN_HEIGHT - 32 - 16 };
	SDL_RenderSetClipRect(renderer, &clip);

	if(canvas->gridShow) {
		rect.x = canvas->x;
		rect.y = canvas->y;
		rect.w = canvas->w * (canvas->pixelSize + 1) + 1;
		rect.h = canvas->h * (canvas->pixelSize + 1) + 1;
		SDL_SetRenderDrawColor(renderer, canvas->palette[canvas->gridColor].r, canvas->palette[canvas->gridColor].g, canvas->palette[canvas->gridColor].b, 255);
		SDL_RenderFillRect(renderer, &rect);
	}

	for(int j = 0; j < canvas->h; j++) {
		for(int i = 0; i < canvas->w; i++) {

			int k = canvas->frame * canvas->w * canvas->h + j * canvas->w + i;
			int l = canvas->pixels[k];

			if(l != canvas->transparent) {
								
				if(canvas->gridShow) {
					rect.x = i * (canvas->pixelSize + 1) + canvas->x + 1;
					rect.y = j * (canvas->pixelSize + 1) + canvas->y + 1;
					rect.w = canvas->pixelSize;
					rect.h = canvas->pixelSize;
				} else {
					rect.x = i * canvas->pixelSize + canvas->x;
					rect.y = j * canvas->pixelSize + canvas->y;
					rect.w = canvas->pixelSize;
					rect.h = canvas->pixelSize;
				}

				SDL_SetRenderDrawColor(renderer, canvas->palette[l].r, canvas->palette[l].g, canvas->palette[l].b, 255);
				SDL_RenderFillRect(renderer, &rect);
			}
		}
	}

	SDL_RenderSetClipRect(renderer, NULL);
}

void Canvas_DrawPoint(Canvas *canvas, int x, int y, byte color) {
	if(x>=0 && x<canvas->w && y>=0 && y<canvas->h) {
		canvas->pixels[canvas->frame*canvas->w*canvas->h+canvas->y*canvas->w+canvas->x]=color;
	}
}

ssize_t Canvas_ReadPoint(Canvas *canvas, int x, int y) {
	if(x>=0 && x<canvas->w && y>=0 && y<canvas->h) {
		return canvas->pixels[canvas->frame*canvas->w*canvas->h+canvas->y*canvas->w+canvas->x];
	}
	return -1;
}
