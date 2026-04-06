/* file: scrollbar.c */

#include <stdlib.h>
#include "scrollbar.h"

static int ScrollBar_GetViewPortSize(ScrollBarOrientation orientation) {
	return orientation == SCROLLBAR_ORIENTATION_VERTICAL
		? 480 - 32 - 16
		: 640 - 16;
}

static int ScrollBar_GetTrackArea(ScrollBarOrientation orientation) {
	return orientation == SCROLLBAR_ORIENTATION_VERTICAL
		? 480 - 32 - 16 - 16 - 16
		: 640 - 16 - 16 - 16;
}

static int ScrollBar_GetContentSize(ScrollBar *scrollBar) {
	if(scrollBar->orientation == SCROLLBAR_ORIENTATION_VERTICAL) {
		return scrollBar->canvas->gridShow
			? scrollBar->canvas->h * (scrollBar->canvas->pixelSize + 1) + 1
			: scrollBar->canvas->h * scrollBar->canvas->pixelSize;
	}

	return scrollBar->canvas->gridShow
		? scrollBar->canvas->w * (scrollBar->canvas->pixelSize + 1) + 1
		: scrollBar->canvas->w * scrollBar->canvas->pixelSize;
}

static int ScrollBar_GetTrackStart(ScrollBar *scrollBar) {
	return scrollBar->orientation == SCROLLBAR_ORIENTATION_VERTICAL
		? scrollBar->y + 16
		: scrollBar->x + 16;
}

static void ScrollBar_UpdateThumbRect(ScrollBar *scrollBar) {
	if(scrollBar->orientation == SCROLLBAR_ORIENTATION_VERTICAL) {
		scrollBar->rectThumb = (SDL_Rect){
			scrollBar->x,
			scrollBar->y + 16 + scrollBar->thumbPosition,
			scrollBar->w,
			scrollBar->thumbSize
		};
	} else {
		scrollBar->rectThumb = (SDL_Rect){
			scrollBar->x + 16 + scrollBar->thumbPosition,
			scrollBar->y,
			scrollBar->thumbSize,
			scrollBar->h
		};
	}
}

static void ScrollBar_ApplyThumb(ScrollBar *scrollBar) {
	int maxScroll = scrollBar->contentSize - scrollBar->viewPortSize;

	if(maxScroll <= 0) {
		scrollBar->scrollPosition = 0;
		scrollBar->thumbPosition = 0;
		scrollBar->contentScrollPosition = 0;
		scrollBar->thumbSize = scrollBar->trackArea;
		if(scrollBar->orientation == SCROLLBAR_ORIENTATION_VERTICAL) scrollBar->canvas->y = 0;
		else scrollBar->canvas->x = 0;
		return;
	}

	scrollBar->scrollPosition = clamp(scrollBar->scrollPosition, 0, maxScroll);

	scrollBar->thumbSize = (int)((double)scrollBar->viewPortSize / scrollBar->contentSize * scrollBar->trackArea);
	scrollBar->thumbSize = clamp(scrollBar->thumbSize, 1, scrollBar->trackArea);

	if(scrollBar->trackArea - scrollBar->thumbSize > 0) {
		scrollBar->thumbPosition =
			(int)((double)scrollBar->scrollPosition / maxScroll * (scrollBar->trackArea - scrollBar->thumbSize));
		scrollBar->contentScrollPosition =
			(int)((double)scrollBar->thumbPosition / (scrollBar->trackArea - scrollBar->thumbSize) * maxScroll);
	} else {
		scrollBar->thumbPosition = 0;
		scrollBar->contentScrollPosition = 0;
	}

	if(scrollBar->orientation == SCROLLBAR_ORIENTATION_VERTICAL) scrollBar->canvas->y = -scrollBar->contentScrollPosition;
	else scrollBar->canvas->x = -scrollBar->contentScrollPosition;
}

static void ScrollBar_BeginDrag(ScrollBar *scrollBar, Mouse *mouse, int trackStart) {
	if(scrollBar->orientation == SCROLLBAR_ORIENTATION_VERTICAL) {
		scrollBar->dragging = 1;
		scrollBar->dragOffset = mouse->y - (trackStart + scrollBar->thumbPosition);
	} else {
		scrollBar->dragging = 1;
		scrollBar->dragOffset = mouse->x - (trackStart + scrollBar->thumbPosition);
	}
}

static void ScrollBar_DoDrag(ScrollBar *scrollBar, Mouse *mouse, int trackStart, int trackLen) {
	int mousePos, newThumbPos, maxScroll;

	maxScroll = scrollBar->contentSize - scrollBar->viewPortSize;
	if(maxScroll <= 0) return;

	mousePos = (scrollBar->orientation == SCROLLBAR_ORIENTATION_VERTICAL) ? mouse->y : mouse->x;

	newThumbPos = clamp(
		mousePos - trackStart - scrollBar->dragOffset,
		0,
		trackLen - scrollBar->thumbSize
	);

	scrollBar->thumbPosition = newThumbPos;
	scrollBar->scrollPosition =
		(int)((double)scrollBar->thumbPosition / (trackLen - scrollBar->thumbSize) * maxScroll);

	scrollBar->contentScrollPosition = scrollBar->scrollPosition;

	if(scrollBar->orientation == SCROLLBAR_ORIENTATION_VERTICAL) {
		scrollBar->canvas->y = -scrollBar->contentScrollPosition;
	} else {
		scrollBar->canvas->x = -scrollBar->contentScrollPosition;
	}
}

static void ScrollBar_Refresh(ScrollBar *scrollBar) {
	scrollBar->viewPortSize = ScrollBar_GetViewPortSize(scrollBar->orientation);
	scrollBar->trackArea = ScrollBar_GetTrackArea(scrollBar->orientation);
	scrollBar->contentSize = ScrollBar_GetContentSize(scrollBar);
	ScrollBar_ApplyThumb(scrollBar);
	ScrollBar_UpdateThumbRect(scrollBar);
}

ScrollBar *ScrollBar_New(
		Palette *palette, Canvas *canvas,
		ScrollBarOrientation orientation,
		int x, int y, int w, int h,
		int buttonSize) {

	ScrollBar *scrollBar = malloc(sizeof(*scrollBar));
	if(!scrollBar) return NULL;

	scrollBar->orientation = orientation;
	scrollBar->x = x;
	scrollBar->y = y;
	scrollBar->w = w;
	scrollBar->h = h;
	scrollBar->buttonSize = buttonSize;
	scrollBar->palette = palette;
	scrollBar->canvas = canvas;
	scrollBar->scrollPosition = 0;
	scrollBar->contentScrollPosition = 0;
	scrollBar->thumbPosition = 0;
	scrollBar->thumbSize = 0;
	scrollBar->viewPortSize = 0;
	scrollBar->contentSize = 0;
	scrollBar->trackArea = 0;
	scrollBar->dragging = 0;
	scrollBar->dragOffset = 0;

	switch(orientation) {
	case SCROLLBAR_ORIENTATION_VERTICAL:
		scrollBar->buttonFirst = Button_New(x, y, w, buttonSize, palette);
		scrollBar->buttonSecond = Button_New(x, y + h - buttonSize, w, buttonSize, palette);
		break;

	case SCROLLBAR_ORIENTATION_HORIZONTAL:
		scrollBar->buttonFirst = Button_New(x, y, buttonSize, h, palette);
		scrollBar->buttonSecond = Button_New(x + w - buttonSize, y, buttonSize, h, palette);
		break;

	default:
		scrollBar->buttonFirst = NULL;
		scrollBar->buttonSecond = NULL;
		break;
	}

	ScrollBar_Refresh(scrollBar);
	return scrollBar;
}

void ScrollBar_Update(ScrollBar *scrollBar, Mouse *mouse) {
	int trackStart, trackLen, thumbStart, thumbEnd;
	SDL_Rect thumbHit;

	ScrollBar_Refresh(scrollBar);

	switch(scrollBar->orientation) {
	case SCROLLBAR_ORIENTATION_VERTICAL:
		if(Button_Update(scrollBar->buttonFirst, mouse)) scrollBar->scrollPosition--;
		if(Button_Update(scrollBar->buttonSecond, mouse)) scrollBar->scrollPosition++;

		trackStart = ScrollBar_GetTrackStart(scrollBar);
		trackLen = scrollBar->trackArea;

		ScrollBar_ApplyThumb(scrollBar);

		thumbHit = (SDL_Rect){
			scrollBar->x,
			trackStart + scrollBar->thumbPosition,
			scrollBar->w,
			scrollBar->thumbSize
		};

		if(mouse->state & SDL_BUTTON(SDL_BUTTON_LEFT)) {
			if(!scrollBar->dragging) {
				if(inrect(mouse->x, mouse->y, thumbHit.x, thumbHit.y, thumbHit.w, thumbHit.h)) {
					ScrollBar_BeginDrag(scrollBar, mouse, trackStart);
				}
			} else {
				ScrollBar_DoDrag(scrollBar, mouse, trackStart, trackLen);
			}
		} else {
			scrollBar->dragging = 0;
		}

		ScrollBar_ApplyThumb(scrollBar);
		ScrollBar_UpdateThumbRect(scrollBar);
		break;

	case SCROLLBAR_ORIENTATION_HORIZONTAL:
		if(Button_Update(scrollBar->buttonFirst, mouse)) scrollBar->scrollPosition--;
		if(Button_Update(scrollBar->buttonSecond, mouse)) scrollBar->scrollPosition++;

		trackStart = ScrollBar_GetTrackStart(scrollBar);
		trackLen = scrollBar->trackArea;

		ScrollBar_ApplyThumb(scrollBar);

		thumbHit = (SDL_Rect){
			trackStart + scrollBar->thumbPosition,
			scrollBar->y,
			scrollBar->thumbSize,
			scrollBar->h
		};

		if(mouse->state & SDL_BUTTON(SDL_BUTTON_LEFT)) {
			if(!scrollBar->dragging) {
				if(inrect(mouse->x, mouse->y, thumbHit.x, thumbHit.y, thumbHit.w, thumbHit.h)) {
					ScrollBar_BeginDrag(scrollBar, mouse, trackStart);
				}
			} else {
				ScrollBar_DoDrag(scrollBar, mouse, trackStart, trackLen);
			}
		} else {
			scrollBar->dragging = 0;
		}

		ScrollBar_ApplyThumb(scrollBar);
		ScrollBar_UpdateThumbRect(scrollBar);
		break;

	default:
		break;
	}
}

void ScrollBar_Draw(ScrollBar *scrollBar, SDL_Renderer *renderer) {

	SDL_Rect clip = { scrollBar->x, scrollBar->y, scrollBar->w, scrollBar->h };
	SDL_RenderSetClipRect(renderer, &clip);

	Button_Draw(scrollBar->buttonFirst, renderer);
	Button_Draw(scrollBar->buttonSecond, renderer);

	SDL_SetRenderDrawColor(renderer,
		scrollBar->palette->colors[13].r,
		scrollBar->palette->colors[13].g,
		scrollBar->palette->colors[13].b,
		255
	);

	SDL_RenderFillRect(renderer, &scrollBar->rectThumb);

	SDL_SetRenderDrawColor(renderer,
		scrollBar->palette->colors[12].r,
		scrollBar->palette->colors[12].g,
		scrollBar->palette->colors[12].b,
		255
	);

	SDL_RenderDrawRect(renderer, &clip);

	SDL_RenderSetClipRect(renderer, NULL);
}


