#include <SDL2/SDL.h>

#include "common.h"
#include "mouse.h"
#include "canvas.h"
#include "palette.h"
#include "button.h"
#include "scrollbar.h"


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

ScrollBar *scrollBarVertical;
ScrollBar *scrollBarHorizontal;

int xScroll,yScroll;
byte currentColor=12;

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

	mouse = Mouse_New("images/mouse.bmp", SCREEN_WIDTH/2, SCREEN_HEIGHT/2, colors[15]);
	SDL_SetCursor(mouse->cursor);
	SDL_WarpMouseInWindow(window, mouse->x, mouse->y);

	canvas=Canvas_New(
				palette,    /* palette     */
				      0,    /* x           */ 
				      0,    /* y           */ 
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
	
	scrollBarVertical = ScrollBar_New(
		palette, canvas,
		SCROLLBAR_ORIENTATION_VERTICAL,
		SCREEN_WIDTH - 16, 0, 
		16, SCREEN_HEIGHT - 32 - 16);

	scrollBarHorizontal = ScrollBar_New(
		palette, canvas,
		SCROLLBAR_ORIENTATION_HORIZONTAL,
		0, SCREEN_HEIGHT - 32 - 16, 
		SCREEN_WIDTH - 16, 16);



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
		ScrollBar_Update(scrollBarVertical, mouse);
		ScrollBar_Update(scrollBarHorizontal, mouse);
		
		Palette_Draw(palette, renderer);
		Canvas_Draw(canvas, renderer);
		ScrollBar_Draw(scrollBarVertical,renderer);
		ScrollBar_Draw(scrollBarHorizontal,renderer);
		
	    SDL_RenderPresent(renderer);
	    
	    SDL_Delay(10);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();

	return 0;
}


