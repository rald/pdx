/* file: mouse.h */

#ifndef MOUSE_H
#define MOUSE_H

#include <SDL2/SDL.h>

typedef struct {
	SDL_Cursor *cursor;
	int x,y;
	Uint32 state;
} Mouse;

Mouse *Mouse_New(char *filename, int x, int y, SDL_Color transparent);
void Mouse_EventHandle(Mouse *mouse, SDL_Event event);
void Mouse_Update(Mouse *mouse);

#endif