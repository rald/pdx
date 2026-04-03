#include <SDL2/SDL.h>

#include "common.h"
#include "canvas.h"
#include "gifenc.h"
#include "gifdec.h"

#define GAME_TITLE "Pixel Dancer"

SDL_Color palette[]={
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

size_t npalette=16;

bool quit = false;
SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
SDL_Event event;

Canvas *canvas = NULL;

int xScroll,yScroll;
byte currentColor=12;

bool inrect(int x, int y, int rx, int ry, int rw, int rh) {
	return x >= rx && x < rx + rw && y >= ry && y < ry + rh;
}

typedef struct {
	SDL_Cursor *cursor;
	int x,y;
	Uint32 state;
} Mouse;

Mouse *mouse = NULL;

Mouse *Mouse_New(char *filename,int x,int y,SDL_Color transparent) {
	Mouse *mouse=malloc(sizeof(*mouse));
	SDL_Surface *surface = NULL;
	if(mouse) {
		mouse->x=x;
		mouse->y=y;

		surface=SDL_LoadBMP(filename);
		SDL_SetColorKey(surface, SDL_TRUE, SDL_MapRGB(surface->format, transparent.r, transparent.g, transparent.b));
		mouse->cursor = SDL_CreateColorCursor(surface, 0, 0); 

		SDL_FreeSurface(surface);
	}
	return mouse;
}

void Mouse_Update(Mouse *mouse) {
	mouse->state=SDL_GetMouseState(&mouse->x,&mouse->y);
}

void Palette_Draw(SDL_Renderer *renderer,SDL_Color *palette,size_t npalette, byte *currentColor, Mouse *mouse) {
	for(int i = 0; i < npalette; i++) {
		
		SDL_Rect rect = {i*32,SCREEN_HEIGHT-32,32,32};
		
		SDL_SetRenderDrawColor(renderer, palette[i].r, palette[i].g, palette[i].b, 255);
		SDL_RenderFillRect(renderer, &rect);

		if((mouse->state & SDL_BUTTON_LMASK) && inrect(mouse->x, mouse->y, rect.x, rect.y, rect.h, rect.w)) {
			*currentColor = i;
		}

		if(i == *currentColor) {
			SDL_SetRenderDrawColor(renderer, palette[12].r, palette[12].g, palette[12].b, 255);
			SDL_RenderDrawRect(renderer, &rect);

			rect.x+=1;
			rect.y+=1;
			rect.w-=2;
			rect.h-=2;

			SDL_SetRenderDrawColor(renderer, palette[0].r, palette[0].g, palette[0].b, 255);
			SDL_RenderDrawRect(renderer, &rect);
		}
	}
}

void ScrollBarVertical_Draw(
		SDL_Renderer *renderer,
		int x, int y, int w, int h, 
		int begin, int end, int value, 
		byte color, SDL_Color *palette, size_t npalette) {

	SDL_Rect clip = { x, y, x + w, y + h };
	SDL_RenderSetClipRect(renderer, &clip);

	SDL_Rect rectUpButton = {x, y, w, 16};	
	SDL_Rect rectDownButton = { x + w - 16, y + h - 16, w, 16};

	SDL_SetRenderDrawColor(renderer, palette[color].r, palette[color].g, palette[color].b, 255);
	SDL_RenderDrawRect(renderer, &rectUpButton);
	SDL_RenderDrawRect(renderer, &rectDownButton);
	
	SDL_RenderSetClipRect(renderer, NULL);
}

void ScrollBarHorizontal_Draw(
		SDL_Renderer *renderer,
		int x, int y, int w, int h, 
		int begin, int end, int value, 
		byte color, SDL_Color *palette, size_t npalette) {

	SDL_Rect clip = { x, y, x + w, y + h };
	SDL_RenderSetClipRect(renderer, &clip);

	SDL_Rect rectLeftButton = {x, y, 16, h};	
	SDL_Rect rectRightButton = { x + w - 16, y + h - 16, 16, h};

	SDL_SetRenderDrawColor(renderer, palette[color].r, palette[color].g, palette[color].b, 255);
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
               
	mouse=Mouse_New("mouse.bmp",SCREEN_WIDTH/2,SCREEN_HEIGHT/2,palette[15]);
	SDL_SetCursor(mouse->cursor);

	canvas=Canvas_New(
           8,    /* x          */ 
           8,    /* y          */ 
          16,    /* w          */
          16,    /* h          */
           4,    /* nframe     */
          -1,    /* tranparent */
           2,    /* color      */
           6,    /* gridColor  */ 
        true,    /* gridShow   */ 
          16,    /* pixelSize  */
           0,    /* frame      */
     palette,    /* palette    */
    npalette     /* npalette   */
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
                case SDLK_g:
                	canvas->gridShow = !canvas->gridShow;
                	break;	
                default: break;
                }
                break;
            case SDL_MOUSEWHEEL:
				xScroll = event.wheel.x; 
				yScroll = event.wheel.y; 
				if (event.wheel.direction == SDL_MOUSEWHEEL_FLIPPED) {
				    xScroll *= -1;
				    yScroll *= -1;
				}
				if(canvas->pixelSize>1 && yScroll<0) {canvas->pixelSize-=1; yScroll=0; }
				if(canvas->pixelSize<32 && yScroll>0) {canvas->pixelSize+=1; yScroll=0; }
            	break;
            default: break;
            }
        }
        
        Mouse_Update(mouse);
	
		SDL_SetRenderDrawColor(renderer, palette[0].r, palette[0].g, palette[0].b, 255);
		SDL_RenderClear(renderer);
		
		Canvas_Draw(canvas,renderer);
		
		ScrollBarVertical_Draw(
			renderer,
			SCREEN_WIDTH - 16, 0, 
			16, SCREEN_HEIGHT - 32 - 16,
			0, 256, 256/2, 
			12, palette, npalette);

		ScrollBarHorizontal_Draw(
			renderer,
			0, SCREEN_HEIGHT - 32 - 16, 
			SCREEN_WIDTH - 16, 16,
			0, 256, 256/2, 
			12, palette, npalette);

		Palette_Draw(renderer, palette, npalette, &currentColor, mouse);
		
	    SDL_RenderPresent(renderer);
	    
	    SDL_Delay(10);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();

	return 0;
}


