#include "mouse.h"

Mouse *Mouse_New(char *filename,int x,int y,SDL_Color transparent) {
	Mouse *mouse=malloc(sizeof(*mouse));
	SDL_Surface *surface = NULL;
	if(mouse) {
		mouse->x=x;
		mouse->y=y;

		surface=SDL_LoadBMP(filename);
		SDL_SetColorKey(surface, SDL_TRUE, SDL_MapRGB(surface->format, transparent.r, transparent.g, transparent.b));
		mouse->cursor = SDL_CreateColorCursor(surface, 0, 0); 

		SDL_FreeSurface(surface);
	}
	return mouse;
}

void Mouse_EventHandle(Mouse *mouse, SDL_Event event) {
}

void Mouse_Update(Mouse *mouse) {
	mouse->state = SDL_GetMouseState(&mouse->x, &mouse->y);
}
