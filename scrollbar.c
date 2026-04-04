


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
		
		if(begin>end) { int tmp=begin; begin=end; end=begin; }

		scrollBar->begin = begin;
		scrollBar->end = end;
		scrollBar->value = value;
		
		scrollBar->color = color;
		scrollBar->palette = palette;
		
		switch(orientation) {
		case SCROLLBAR_ORIENTATION_VERTICAL:
			scrollBar->buttonUp = Button_New(x, y, w, 16, palette);
			scrollBar->buttonDown = Button_New(x + w - 16, y + h - 16, w, 16, palette);
			scrollBar->rectThumb = (SDL_Rect) {x, y + value + 16 , w, 16};			
			break;
		case SCROLLBAR_ORIENTATION_HORIZONTAL:
			scrollBar->buttonLeft = Button_New(x, y, 16, h, palette);
			scrollBar->buttonRight = Button_New(x + w - 16, y + h - 16, 16, h, palette);
			scrollBar->rectThumb = (SDL_Rect) {x + value + 16, y , 16, h};
			break;
		}

	}	
	
	return scrollBar;
}

void ScrollBar_Draw(ScrollBar *scrollBar, SDL_Renderer *renderer) {

	SDL_Rect clip = { scrollBar->x, scrollBar->y, scrollBar->w, scrollBar->h };
	SDL_RenderSetClipRect(renderer, &clip);
	
	SDL_SetRenderDrawColor(renderer,
		scrollBar->palette->colors[12].r,
		scrollBar->palette->colors[12].g,
		scrollBar->palette->colors[12].b,
		255
	);
		
	SDL_RenderDrawRect(renderer, &clip);

	switch(scrollBar->orientation) {
	case SCROLLBAR_ORIENTATION_VERTICAL:
		Button_Draw(scrollBar->buttonUp, renderer);
		Button_Draw(scrollBar->buttonDown, renderer);
		break;
	case SCROLLBAR_ORIENTATION_HORIZONTAL:
		Button_Draw(scrollBar->buttonLeft, renderer);
		Button_Draw(scrollBar->buttonRight, renderer);
		break;
	default: break;
	}

	SDL_SetRenderDrawColor(renderer,
		scrollBar->palette->colors[12].r,
		scrollBar->palette->colors[12].g,
		scrollBar->palette->colors[12].b,
		255
	);
		
	SDL_RenderDrawRect(renderer, &scrollBar->rectThumb);

	SDL_RenderSetClipRect(renderer, NULL);
}

void ScrollBar_Update(ScrollBar *scrollBar, Mouse *mouse) {
	switch(scrollBar->orientation) {
	case SCROLLBAR_ORIENTATION_VERTICAL:
		if(Button_Update(scrollBar->buttonUp, mouse)) {
			if(scrollBar->value > scrollBar->begin) scrollBar->value--;
		}
		if(Button_Update(scrollBar->buttonDown, mouse)) {
			if(scrollBar->value < scrollBar->end) scrollBar->value++;
		}
		scrollBar->value = clamp(scrollBar->value,scrollBar->begin,scrollBar->end);
		scrollBar->rectThumb.y = (double) scrollBar->value / (scrollBar->end - scrollBar->begin) * (scrollBar->h - 16 * 3) + 16;
		break;
	case SCROLLBAR_ORIENTATION_HORIZONTAL:
		if(Button_Update(scrollBar->buttonLeft, mouse)) {
			if(scrollBar->value > scrollBar->begin) scrollBar->value--;
		}
		if(Button_Update(scrollBar->buttonRight, mouse)) {
			if(scrollBar->value < scrollBar->end) scrollBar->value++;
		}
		scrollBar->value = clamp(scrollBar->value,scrollBar->begin,scrollBar->end);
		scrollBar->rectThumb.x = (double) scrollBar->value / (scrollBar->end - scrollBar->begin) * (scrollBar->w - 16 * 3) + 16;
		break;			
	default: break;
	}
}


