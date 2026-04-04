#ifndef BUTTON_H
#define BUTTON_H

#include <SDL2/SDL.h>

#include "palette.h"

typedef enum {
	BUTTON_STATE_UP = 0,
	BUTTON_STATE_OVER,
	BUTTON_STATE_DOWN,
	BUTTON_STATE_MAX
} ButtonState;

typedef struct {
	ButtonState state;
	int x,y,w,h;
	Palette *palette;
} Button;

Button *Button_New(int x, int y, int w, int h, Palette *palette);
void Button_Draw(Button *button, SDL_Renderer *renderer);
bool Button_Update(Button *button, Mouse *mouse);

#endif