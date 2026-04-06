/* file: scrollbar.h */

#ifndef SCROLLBAR_H
#define SCROLLBAR_H

#include <SDL2/SDL.h>

#include "types.h"
#include "palette.h"
#include "canvas.h"
#include "button.h"

typedef enum {
	SCROLLBAR_ORIENTATION_VERTICAL = 0,
	SCROLLBAR_ORIENTATION_HORIZONTAL,
	SCROLLBAR_ORIENTATION_MAX
} ScrollBarOrientation;

typedef struct {
	ScrollBarOrientation orientation;
	int x,y,w,h;
	int buttonSize;
	Palette *palette;
	Canvas *canvas;

	Button *buttonFirst;
	Button *buttonSecond;
	SDL_Rect rectThumb;

	int thumbSize;
	int viewPortSize;
	int contentSize;
	int trackArea;
	int thumbPosition;
	int scrollPosition;
	int contentScrollPosition;
	int dragging;
	int dragOffset;
} ScrollBar;

ScrollBar *ScrollBar_New(
		Palette *palette, Canvas *canvas,
		ScrollBarOrientation orientation,
		int x, int y, int w, int h,
		int buttonSize);

void ScrollBar_Draw(ScrollBar *scrollbar, SDL_Renderer *renderer);
void ScrollBar_Update(ScrollBar *scrollbar, Mouse *mouse);

#endif


