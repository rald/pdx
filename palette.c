#include "palette.h"

Palette *Palette_New(SDL_Color *colors, size_t ncolors, int x, int y, int w, int h, byte currentColor, size_t boxSize) {
	Palette *palette = malloc(sizeof(*palette));
	if(palette) {
		palette->colors = colors;
		palette->ncolors = ncolors;
		palette->x = x;
		palette->y = y;
		palette->w = w;
		palette->h = h;
		palette->currentColor = currentColor;
		palette->boxSize = boxSize;
	}
	return palette;
}

void Palette_EventHandle(Palette *palette, SDL_Event event) {
}

void Palette_Update(Palette *palette, Mouse *mouse) {
	if(mouse->state & SDL_BUTTON_LMASK) {
		if(inrect(mouse->x,mouse->y,palette->x,palette->y,palette->w,palette->h)) {
			byte i=(mouse->x - palette->x) / palette->boxSize;
			if(i>=0 && i<=palette->ncolors) {
				palette->currentColor=i;
			}
		}
	}
}

void Palette_Draw(Palette *palette, SDL_Renderer *renderer) {

	SDL_Rect clip = { palette->x, palette->y, palette->w, palette->h};
	SDL_RenderSetClipRect(renderer, &clip);

	for(int i = 0; i < palette->ncolors; i++) {
		
		SDL_Rect rect = {i*32,SCREEN_HEIGHT-32,32,32};
		
		SDL_SetRenderDrawColor(renderer, palette->colors[i].r, palette->colors[i].g, palette->colors[i].b, 255);
		SDL_RenderFillRect(renderer, &rect);

		if(i == palette->currentColor) {
			SDL_SetRenderDrawColor(renderer, palette->colors[0].r, palette->colors[0].g, palette->colors[0].b, 255);
			SDL_RenderFillRect(renderer, &rect);

			rect.x+=4;
			rect.y+=4;
			rect.w-=8;
			rect.h-=8;

			SDL_SetRenderDrawColor(renderer, palette->colors[i].r, palette->colors[i].g, palette->colors[i].b, 255);
			SDL_RenderFillRect(renderer, &rect);

			SDL_SetRenderDrawColor(renderer, palette->colors[12].r, palette->colors[12].g, palette->colors[12].b, 255);
			SDL_RenderDrawRect(renderer, &rect);
		}
	}

	SDL_Rect rect = {palette->x, palette->y, palette->w, palette->h};
	SDL_SetRenderDrawColor(renderer, palette->colors[12].r, palette->colors[12].g, palette->colors[12].b, 255);
	SDL_RenderDrawRect(renderer, &rect);
	
	SDL_RenderSetClipRect(renderer, NULL);
}

