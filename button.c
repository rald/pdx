#include "button.h"

Button *Button_New(int x, int y, int w, int h, Palette *palette) {
	Button *button = malloc(sizeof(*button));
	if(button) {
		button->state = BUTTON_STATE_UP;
		button->x = x;
		button->y = y;
		button->w = w;
		button->h = h;
		button->palette = palette;
	}
	return button;
}

void Button_Draw(Button *button, SDL_Renderer *renderer) {
	SDL_Rect rect = {button->x, button->y, button->w, button->h};
	switch(button->state) {
	case BUTTON_STATE_UP:
		SDL_SetRenderDrawColor(
			renderer,
			button->palette->colors[12].r,
			button->palette->colors[12].g,
			button->palette->colors[12].b,
			255);
		SDL_RenderDrawRect(renderer, &rect);
		break;
	case BUTTON_STATE_OVER:
		SDL_SetRenderDrawColor(
			renderer,
			button->palette->colors[12].r,
			button->palette->colors[12].g,
			button->palette->colors[12].b,
			255);
		SDL_RenderFillRect(renderer, &rect);
		break;
	case BUTTON_STATE_DOWN: 
		SDL_SetRenderDrawColor(
			renderer,
			button->palette->colors[12].r,
			button->palette->colors[12].g,
			button->palette->colors[12].b,
			255);
		SDL_RenderDrawRect(renderer, &rect);
		break;
	default: break;
	}
}

bool Button_Update(Button *button, Mouse *mouse) {
	SDL_Rect rect = {button->x, button->y, button->w, button->h};
	if(inrect(mouse->x,mouse->y,button->x,button->y,button->w,button->h)) {
		if(mouse->state & SDL_BUTTON_LMASK) {
			button->state=BUTTON_STATE_DOWN;
			return true;
		} else {
			button->state=BUTTON_STATE_OVER;
		}		
	} else {
		button->state=BUTTON_STATE_UP;	
	}		
	return false;
}


