/* file: main.c */

#include <SDL2/SDL.h>

#include "common.h"
#include "mouse.h"
#include "canvas.h"
#include "palette.h"
#include "target.h"
#include "button.h"
#include "scrollbar.h"
#include "MyWindow.h"


#include "gifenc.h"
#include "gifdec.h"

#define GAME_TITLE "Pixel Dancer"

#define CANVAS_WIDTH   64
#define CANVAS_HEIGHT  64
#define CANVAS_NFRAME   4



SDL_Color colors[]={
    { 26, 28, 44},
    { 93, 39, 93},
    {177, 62, 83},
    {239,125, 87},
    {255,205,117},
    {167,240,112},
    { 56,183,100},
    { 37,113,121},
    { 41, 54,111},
    { 59, 93,201},
    { 65,166,246},
    {115,239,247},
    {244,244,244},
    {148,176,194},
    { 86,108,134},
    { 51, 60, 87},
};

size_t ncolors=16;

bool quit = false;
SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
SDL_Event event;

Canvas *canvas = NULL;
Mouse *mouse = NULL;
Palette *palette = NULL;
Target *target = NULL;

MyWindow *myWindow = NULL;

int main(int argc,char *argv[]) {

	SDL_Init(SDL_INIT_VIDEO);

    window = SDL_CreateWindow(GAME_TITLE,
             SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
             SCREEN_WIDTH, SCREEN_HEIGHT,
             SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

    renderer = SDL_CreateRenderer(window, -1,
               SDL_RENDERER_ACCELERATED |
               SDL_RENDERER_TARGETTEXTURE);
               
               
	palette = Palette_New(colors, ncolors, 0, SCREEN_HEIGHT - 32, 32 * ncolors, 32, 0, 32);

	mouse = Mouse_New("images/mouse.bmp", SCREEN_WIDTH/2, SCREEN_HEIGHT/2, colors[15]);
	SDL_SetCursor(mouse->cursor);
	SDL_WarpMouseInWindow(window, mouse->x, mouse->y);


	canvas=Canvas_LoadCVS("cvs/dog.cvs", palette);
	
	target=Target_New(palette,canvas,0,0);

	myWindow = MyWindow_New(palette, canvas, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT - 32, 16);
	
    while(!quit) {
		
		bool overPalette = inrect(mouse->x, mouse->y, palette->x, palette->y, palette->w, palette->h);

		bool overScrollBar =
			inrect(mouse->x, mouse->y, myWindow->vscroll->x, myWindow->vscroll->y, myWindow->vscroll->w, myWindow->vscroll->h) ||
			inrect(mouse->x, mouse->y, myWindow->hscroll->x, myWindow->hscroll->y, myWindow->hscroll->w, myWindow->hscroll->h);

		bool draggingScrollBar =
			myWindow->vscroll->dragging ||
			myWindow->hscroll->dragging;

        while(SDL_PollEvent(&event)) {
            switch(event.type) {
            case SDL_QUIT:
                quit=true;
                break;
            case SDL_KEYDOWN:
                switch(event.key.keysym.sym) {
                case SDLK_ESCAPE:
                    quit = true;
                    break;
                default: break;
                }
                break;
            default: break;
            }
            
            Mouse_EventHandle(mouse,event);

			if(!palette->scrubbing && !overPalette && !overScrollBar && !draggingScrollBar) {
				Canvas_EventHandle(canvas, event);
			}
        }
        	
		SDL_SetRenderDrawColor(renderer, palette->colors[0].r, palette->colors[0].g, palette->colors[0].b, 255);
		SDL_RenderClear(renderer);

        Mouse_Update(mouse);
		
		if(!palette->scrubbing && !overPalette) {
			MyWindow_Update(myWindow, mouse);
		}

		if(!overScrollBar && !draggingScrollBar) {
			Palette_Update(palette, mouse);
		}

		Target_Update(target, mouse);
		
		MyWindow_Draw(myWindow, renderer);
		Palette_Draw(palette, renderer);
		Target_Draw(target, renderer);

	    SDL_RenderPresent(renderer);
	    
	    SDL_Delay(10);
    }

	MyWindow_Free(myWindow);
	Palette_Free(palette);
	Canvas_Free(canvas);
	Target_Free(target);
	Mouse_Free(mouse);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();

	return 0;
}


