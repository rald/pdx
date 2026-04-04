#include "scrollbar.h"
#include <stdlib.h>

static void ScrollBar_Apply(ScrollBar *scrollBar) {
	int maxScroll = scrollBar->contentSize - scrollBar->viewPortSize;

	if(maxScroll <= 0) {
		scrollBar->scrollPosition = 0;
		scrollBar->thumbPosition = 0;
		scrollBar->contentScrollPosition = 0;
		scrollBar->thumbSize = scrollBar->trackArea;
		if(scrollBar->orientation == SCROLLBAR_ORIENTATION_VERTICAL) {
			scrollBar->canvas->y = 0;
			scrollBar->rectThumb = (SDL_Rect){ scrollBar->x, scrollBar->y + 16, scrollBar->w, scrollBar->thumbSize };
		} else {
			scrollBar->canvas->x = 0;
			scrollBar->rectThumb = (SDL_Rect){ scrollBar->x + 16, scrollBar->y, scrollBar->thumbSize, scrollBar->h };
		}
		return;
	}

	scrollBar->scrollPosition = clamp(scrollBar->scrollPosition, 0, maxScroll);

	scrollBar->thumbSize = (int)((double)scrollBar->viewPortSize / scrollBar->contentSize * scrollBar->trackArea);
	if(scrollBar->thumbSize < 1) scrollBar->thumbSize = 1;
	if(scrollBar->thumbSize > scrollBar->trackArea) scrollBar->thumbSize = scrollBar->trackArea;

	if(scrollBar->trackArea - scrollBar->thumbSize > 0) {
		scrollBar->thumbPosition = (int)((double)scrollBar->scrollPosition / maxScroll * (scrollBar->trackArea - scrollBar->thumbSize));
		scrollBar->contentScrollPosition = (int)((double)scrollBar->thumbPosition / (scrollBar->trackArea - scrollBar->thumbSize) * maxScroll);
	} else {
		scrollBar->thumbPosition = 0;
		scrollBar->contentScrollPosition = 0;
	}

	if(scrollBar->orientation == SCROLLBAR_ORIENTATION_VERTICAL) {
		scrollBar->rectThumb = (SDL_Rect){ scrollBar->x, scrollBar->y + 16 + scrollBar->thumbPosition, scrollBar->w, scrollBar->thumbSize };
		scrollBar->canvas->y = -scrollBar->contentScrollPosition;
	} else {
		scrollBar->rectThumb = (SDL_Rect){ scrollBar->x + 16 + scrollBar->thumbPosition, scrollBar->y, scrollBar->thumbSize, scrollBar->h };
		scrollBar->canvas->x = -scrollBar->contentScrollPosition;
	}
}

ScrollBar *ScrollBar_New(
		Palette *palette, Canvas *canvas,
		ScrollBarOrientation orientation,
		int x, int y, int w, int h) {

	ScrollBar *scrollBar = malloc(sizeof(*scrollBar));
	if(!scrollBar) return NULL;

	scrollBar->orientation = orientation;
	scrollBar->x = x;
	scrollBar->y = y;
	scrollBar->w = w;
	scrollBar->h = h;
	scrollBar->palette = palette;
	scrollBar->canvas = canvas;

	scrollBar->scrollPosition = 0;
	scrollBar->contentScrollPosition = 0;
	scrollBar->thumbPosition = 0;
	scrollBar->thumbSize = 0;
	scrollBar->viewPortSize = 0;
	scrollBar->contentSize = 0;
	scrollBar->trackArea = 0;

	switch(orientation) {
	case SCROLLBAR_ORIENTATION_VERTICAL:
		scrollBar->buttonUp = Button_New(x, y, w, 16, palette);
		scrollBar->buttonDown = Button_New(x, y + h - 16, w, 16, palette);
		scrollBar->buttonLeft = NULL;
		scrollBar->buttonRight = NULL;
		scrollBar->viewPortSize = 480 - 32 - 16;
		scrollBar->trackArea = 480 - 32 - 16 - 16 - 16;
		scrollBar->contentSize = canvas->gridShow
			? canvas->h * (canvas->pixelSize + 1) + 1
			: canvas->h * canvas->pixelSize;
		break;

	case SCROLLBAR_ORIENTATION_HORIZONTAL:
		scrollBar->buttonLeft = Button_New(x, y, 16, h, palette);
		scrollBar->buttonRight = Button_New(x + w - 16, y, 16, h, palette);
		scrollBar->buttonUp = NULL;
		scrollBar->buttonDown = NULL;
		scrollBar->viewPortSize = 640 - 16;
		scrollBar->trackArea = 640 - 16 - 16 - 16;
		scrollBar->contentSize = canvas->gridShow
			? canvas->w * (canvas->pixelSize + 1) + 1
			: canvas->w * canvas->pixelSize;
		break;

	default:
		scrollBar->buttonUp = NULL;
		scrollBar->buttonDown = NULL;
		scrollBar->buttonLeft = NULL;
		scrollBar->buttonRight = NULL;
		break;
	}

	ScrollBar_Apply(scrollBar);
	return scrollBar;
}

void ScrollBar_Update(ScrollBar *scrollBar, Mouse *mouse) {
	switch(scrollBar->orientation) {
	case SCROLLBAR_ORIENTATION_VERTICAL:
		if(Button_Update(scrollBar->buttonUp, mouse)) scrollBar->scrollPosition--;
		if(Button_Update(scrollBar->buttonDown, mouse)) scrollBar->scrollPosition++;

		scrollBar->viewPortSize = 480 - 32 - 16;
		scrollBar->trackArea = 480 - 32 - 16 - 16 - 16;
		scrollBar->contentSize = scrollBar->canvas->gridShow
			? scrollBar->canvas->h * (scrollBar->canvas->pixelSize + 1) + 1
			: scrollBar->canvas->h * scrollBar->canvas->pixelSize;

		ScrollBar_Apply(scrollBar);
		break;

	case SCROLLBAR_ORIENTATION_HORIZONTAL:
		if(Button_Update(scrollBar->buttonLeft, mouse)) scrollBar->scrollPosition--;
		if(Button_Update(scrollBar->buttonRight, mouse)) scrollBar->scrollPosition++;

		scrollBar->viewPortSize = 640 - 16;
		scrollBar->trackArea = 640 - 16 - 16 - 16;
		scrollBar->contentSize = scrollBar->canvas->gridShow
			? scrollBar->canvas->w * (scrollBar->canvas->pixelSize + 1) + 1
			: scrollBar->canvas->w * scrollBar->canvas->pixelSize;

		ScrollBar_Apply(scrollBar);
		break;

	default:
		break;
	}
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
	default:
		break;
	}

	SDL_RenderDrawRect(renderer, &scrollBar->rectThumb);

	SDL_RenderSetClipRect(renderer, NULL);
}


