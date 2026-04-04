#include <SDL2/SDL.h>

#include "common.h"
#include "mouse.h"
#include "canvas.h"
#include "palette.h"

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

int xScroll,yScroll;
byte currentColor=12;

void ScrollBarVertical_Draw(
		SDL_Renderer *renderer,
		Palette *palette,
		int x, int y, int w, int h, 
		int begin, int end, int value, 
		byte color) {

	SDL_Rect clip = { x, y, x + w, y + h };
	SDL_RenderSetClipRect(renderer, &clip);

	SDL_Rect rectUpButton = {x, y, w, 16};	
	SDL_Rect rectDownButton = { x + w - 16, y + h - 16, w, 16};

	SDL_SetRenderDrawColor(renderer, palette->colors[color].r, palette->colors[color].g, palette->colors[color].b, 255);
	SDL_RenderDrawRect(renderer, &rectUpButton);
	SDL_RenderDrawRect(renderer, &rectDownButton);
	
	SDL_RenderSetClipRect(renderer, NULL);
}

void ScrollBarHorizontal_Draw(
		SDL_Renderer *renderer,
		Palette *palette,
		int x, int y, int w, int h, 
		int begin, int end, int value, 
		byte color) {

	SDL_Rect clip = { x, y, x + w, y + h };
	SDL_RenderSetClipRect(renderer, &clip);

	SDL_Rect rectLeftButton = {x, y, 16, h};	
	SDL_Rect rectRightButton = { x + w - 16, y + h - 16, 16, h};

	SDL_SetRenderDrawColor(renderer, palette->colors[color].r, palette->colors[color].g, palette->colors[color].b, 255);
	SDL_RenderDrawRect(renderer, &rectLeftButton);
	SDL_RenderDrawRect(renderer, &rectRightButton);
	
	SDL_RenderSetClipRect(renderer, NULL);
}


int main(int argc,char *argv[]) {

	SDL_Init(SDL_INIT_VIDEO);

    window = SDL_CreateWindow(GAME_TITLE,
             SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
             SCREEN_WIDTH, SCREEN_HEIGHT,
             SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

    renderer = SDL_CreateRenderer(window, -1,
               SDL_RENDERER_ACCELERATED |
               SDL_RENDERER_TARGETTEXTURE);
               
               
	palette = Palette_New(colors, ncolors, 0, SCREEN_HEIGHT - 32, 32 * ncolors, 32, 12, 32);

	mouse = Mouse_New("images/mouse.bmp",SCREEN_WIDTH/2,SCREEN_HEIGHT/2,colors[15]);
	SDL_SetCursor(mouse->cursor);

	canvas=Canvas_New(
                                palette,    /* palette     */
       (SCREEN_WIDTH-CANVAS_WIDTH-16)/2,    /* x           */ 
  (SCREEN_HEIGHT-CANVAS_HEIGHT-32-16)/2,    /* y           */ 
                           CANVAS_WIDTH,    /* w           */
                          CANVAS_HEIGHT,    /* h           */
                          CANVAS_NFRAME,    /* nframe      */
                                     -1,    /* transparent */
                                     12,    /* color       */
                                      6,    /* gridColor   */ 
                                  false,    /* gridShow    */ 
                                      1,    /* pixelSize   */
                                      0    /* frame       */
	);

    while(!quit) {

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
            Canvas_EventHandle(canvas,event);
            Palette_EventHandle(palette,event);
        }
        
	
		SDL_SetRenderDrawColor(renderer, palette->colors[0].r, palette->colors[0].g, palette->colors[0].b, 255);
		SDL_RenderClear(renderer);

        Mouse_Update(mouse);
		Palette_Update(palette, mouse);	
		Canvas_Update(canvas, mouse);	
		
		Canvas_Draw(canvas,renderer);
		
		ScrollBarVertical_Draw(
			renderer,
			palette,
			SCREEN_WIDTH - 16, 0, 
			16, SCREEN_HEIGHT - 32 - 16,
			0, 256, 256/2, 
			12);

		ScrollBarHorizontal_Draw(
			renderer,
			palette,
			0, SCREEN_HEIGHT - 32 - 16, 
			SCREEN_WIDTH - 16, 16,
			0, 256, 256/2, 
			12);

		Palette_Draw(palette, renderer, mouse);
		
	    SDL_RenderPresent(renderer);
	    
	    SDL_Delay(10);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();

	return 0;
}


