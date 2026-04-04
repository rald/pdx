#include "scrollbar.h"

ScrollBar *ScrollBar_New(
		Palette *palette, Canvas *canvas,
		ScrollBarOrientation orientation,
		int x, int y, int w, int h) {
	
	ScrollBar *scrollBar = malloc(sizeof(*scrollBar));
	
	if(scrollBar) {
		scrollBar->orientation = orientation;
		scrollBar->x = x;
		scrollBar->y = y;
		scrollBar->w = w;
		scrollBar->h = h;
		
		scrollBar->palette = palette;
		scrollBar->canvas = canvas;
						
		switch(orientation) {
		case SCROLLBAR_ORIENTATION_VERTICAL:
			scrollBar->buttonUp = Button_New(x, y, w, 16, palette);
			scrollBar->buttonDown = Button_New(x + w - 16, y + h - 16, w, 16, palette);
			scrollBar->scrollPosition = 0;
			scrollBar->contentScrollPosition = 0;
			break;
		case SCROLLBAR_ORIENTATION_HORIZONTAL:
			scrollBar->buttonLeft = Button_New(x, y, 16, h, palette);
			scrollBar->buttonRight = Button_New(x + w - 16, y + h - 16, 16, h, palette);
			scrollBar->scrollPosition = 0;
			scrollBar->contentScrollPosition = 0;
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
			scrollBar->scrollPosition--;
		}
		if(Button_Update(scrollBar->buttonDown, mouse)) {
			scrollBar->scrollPosition++;
		}
		scrollBar->viewPortSize = 480-32-16;
		scrollBar->trackArea = 480-32-16-16-16;			
		scrollBar->contentSize = scrollBar->canvas->gridShow ? scrollBar->canvas->h * (scrollBar->canvas->pixelSize + 1) + 1 : scrollBar->canvas->h * scrollBar->canvas->pixelSize;

		scrollBar->scrollPosition = clamp(scrollBar->scrollPosition, 0, scrollBar->contentSize - scrollBar->viewPortSize);
		scrollBar->thumbSize = (int)((double) scrollBar->viewPortSize / scrollBar->contentSize * scrollBar->trackArea);
		scrollBar->thumbPosition = (double) scrollBar->scrollPosition / (scrollBar->contentSize - scrollBar->viewPortSize) * (scrollBar->trackArea - scrollBar->thumbSize);		
		scrollBar->rectThumb = (SDL_Rect) {scrollBar->x, scrollBar->y + scrollBar->thumbPosition + 16, scrollBar->w, scrollBar->thumbSize };

		scrollBar->contentScrollPosition = (int)((double)scrollBar->thumbPosition / (scrollBar->trackArea - scrollBar->thumbSize) * (scrollBar->contentSize - scrollBar->viewPortSize));

		scrollBar->canvas->y = -scrollBar->contentScrollPosition;
		
		break;
	case SCROLLBAR_ORIENTATION_HORIZONTAL:
		if(Button_Update(scrollBar->buttonLeft, mouse)) {
			scrollBar->scrollPosition--;
		}
		if(Button_Update(scrollBar->buttonRight, mouse)) {
			scrollBar->scrollPosition++;
		}
		scrollBar->viewPortSize = 640-16;
		scrollBar->contentSize = scrollBar->canvas->gridShow ? scrollBar->canvas->w * (scrollBar->canvas->pixelSize + 1) + 1 : scrollBar->canvas->w * scrollBar->canvas->pixelSize;
		scrollBar->trackArea = 640-16-16-16;

		scrollBar->scrollPosition = clamp(scrollBar->scrollPosition, 0, scrollBar->contentSize - scrollBar->viewPortSize);
		scrollBar->thumbSize = (int)((double) scrollBar->viewPortSize / scrollBar->contentSize * scrollBar->trackArea);
		scrollBar->thumbPosition = (double) scrollBar->scrollPosition / (scrollBar->contentSize - scrollBar->viewPortSize) * (scrollBar->trackArea - scrollBar->thumbSize);		
		scrollBar->rectThumb = (SDL_Rect) {scrollBar->x + scrollBar->thumbPosition + 16, scrollBar->y, scrollBar->thumbSize, scrollBar->h };

		scrollBar->contentScrollPosition = (int)((double)scrollBar->thumbPosition / (scrollBar->trackArea - scrollBar->thumbSize) * (scrollBar->contentSize - scrollBar->viewPortSize));

		scrollBar->canvas->x = -scrollBar->contentScrollPosition;
		break;			
	default: break;
	}
}


