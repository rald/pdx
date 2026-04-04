#include "scrollbar.h"

ScrollBar *ScrollBar_New(
		ScrollBarOrientation orientation,
		int x, int y, int w, int h, 
		int begin, int end, int value, 
		byte color, Palette *palette) {
	
	ScrollBar *scrollBar = malloc(sizeof(*scrollBar));
	
	if(scrollBar) {
		scrollBar->orientation = orientation;
		scrollBar->x = x;
		scrollBar->y = y;
		scrollBar->w = w;
		scrollBar->h = h;
		scrollBar->begin = begin;
		scrollBar->end = end;
		scrollBar->value = value;
		scrollBar->color = color;
		scrollBar->palette = palette;
		
		switch(orientation) {
		case SCROLLBAR_ORIENTATION_VERTICAL:
			scrollBar->buttonUp = Button_New(x, y, w, 16, palette);
			scrollBar->buttonDown = Button_New(x + w - 16, y + h - 16, w, 16, palette);
			scrollBar->buttonThumb = Button_New(x, y + value + 16 , w, 16, palette);			
			break;
		case SCROLLBAR_ORIENTATION_HORIZONTAL:
			scrollBar->buttonLeft = Button_New(x, y, 16, h, palette);
			scrollBar->buttonRight = Button_New(x + w - 16, y + h - 16, 16, h, palette);
			scrollBar->buttonThumb = Button_New(x + value + 16, y , 16, h, palette);
			break;
		}

	}	
	
	return scrollBar;
}

void ScrollBar_Draw(ScrollBar *scrollBar, SDL_Renderer *renderer) {

	SDL_Rect clip = { scrollBar->x, scrollBar->y, scrollBar->x + scrollBar->w, scrollBar->y + scrollBar->h };
	SDL_RenderSetClipRect(renderer, &clip);

	switch(scrollBar->orientation) {
	case SCROLLBAR_ORIENTATION_VERTICAL:
		Button_Draw(scrollBar->buttonUp, renderer);
		Button_Draw(scrollBar->buttonDown, renderer);
		Button_Draw(scrollBar->buttonThumb, renderer);
		break;
	case SCROLLBAR_ORIENTATION_HORIZONTAL:
		Button_Draw(scrollBar->buttonLeft, renderer);
		Button_Draw(scrollBar->buttonRight, renderer);
		Button_Draw(scrollBar->buttonThumb, renderer);
		break;
	default: break;
	}

	SDL_RenderSetClipRect(renderer, NULL);
}

void ScrollBar_Update(ScrollBar *scrollBar, Mouse *mouse) {
	switch(scrollBar->orientation) {
	case SCROLLBAR_ORIENTATION_VERTICAL:
		if(Button_Update(scrollBar->buttonUp, mouse)) {
			if(scrollBar->buttonThumb->y > 16) scrollBar->buttonThumb->y--;
		}
		if(Button_Update(scrollBar->buttonDown, mouse)) {
			if(scrollBar->buttonThumb->y < scrollBar->h - 16 * 2) scrollBar->buttonThumb->y++;
		}
		Button_Update(scrollBar->buttonThumb, mouse);
		break;
	case SCROLLBAR_ORIENTATION_HORIZONTAL:
		if(Button_Update(scrollBar->buttonLeft, mouse)) {
			if(scrollBar->buttonThumb->x > 16) scrollBar->buttonThumb->x--;
		}
		if(Button_Update(scrollBar->buttonRight, mouse)) {
			if(scrollBar->buttonThumb->x < scrollBar->w - 16 * 2) scrollBar->buttonThumb->x++;
		}
		Button_Update(scrollBar->buttonThumb, mouse);
		break;			
	default: break;
	}
}
