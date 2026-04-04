#ifndef SCROLLBAR_H
#define SCROLLBAR_H

#include <SDL2/SDL.h>

#include "types.h"
#include "palette.h"
#include "button.h"

typedef enum {
	SCROLLBAR_ORIENTATION_VERTICAL = 0,
	SCROLLBAR_ORIENTATION_HORIZONTAL,
	SCROLLBAR_ORIENTATION_MAX
} ScrollBarOrientation;

typedef struct {
	ScrollBarOrientation orientation;
	int x,y,w,h;
	int begin,end,value;
	byte color;
	Palette *palette;

	Button *buttonUp;
	Button *buttonDown;
	Button *buttonLeft;
	Button *buttonRight;
	SDL_Rect rectThumb;

} ScrollBar;

ScrollBar *ScrollBar_New(
		ScrollBarOrientation orientation,
		int x, int y, int w, int h, 
		int begin, int end, int value, 
		byte color,
		Palette *palette);

void ScrollBar_Draw(ScrollBar *scrollbar, SDL_Renderer *renderer);
void ScrollBar_Update(ScrollBar *scrollbar, Mouse *mouse);

#endif
