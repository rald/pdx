#include "target.h"

Target *Target_New(Palette *palette, Canvas *canvas, int x, int y) {
	Target *target = malloc(sizeof(*target));
	if(target) {
		target->palette=palette;
		target->canvas=canvas;
		target->x = x;
		target->y = y;
	}
	return target;
}

void Target_Draw(Target *target,SDL_Renderer *renderer) {
	SDL_SetRenderDrawColor(renderer,
		target->palette->colors[0].r,
		target->palette->colors[0].g,
		target->palette->colors[0].b,
		255
	);

	SDL_RenderDrawLine(renderer,target->x-4,target->y,target->x+4,target->y);
	SDL_RenderDrawLine(renderer,target->x,target->y-4,target->x,target->y+4);
}

void Target_Update(Target *target, Mouse *mouse) {
	if(mouse->state & SDL_BUTTON_RMASK) {
		target->x = mouse->x;
		target->y = mouse->y;
	}
}