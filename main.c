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
#include "history.h"


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
History *history = NULL;
Mouse *mouse = NULL;
Palette *palette = NULL;
Target *target = NULL;

MyWindow *myWindow = NULL;

Canvas *font = NULL;

bool isDrawing = false;
int dx = 0, dy = 0;
int cx = 0, cy = 0;
int tx = 0, ty = 0;
byte oldColor,newColor;




void DrawChar(SDL_Renderer *renderer, Canvas *font, Palette *palette, int x, int y, int pixelSize, byte ch) {
	int i, j, k;
	for(j = 0; j < font->h; j++) {
		for(i = 0; i < font->w; i++) {

			k = font->pixels[(ch - 32) * font->w * font->h + j * font->w + i];

			if(k==font->transparent) continue;

			SDL_SetRenderDrawColor(renderer,
				palette->colors[k].r,
				palette->colors[k].g,
				palette->colors[k].b,
				255
			);

			SDL_Rect rect = {
				i * pixelSize+x, j * pixelSize+y,
				pixelSize, pixelSize
			};

			SDL_RenderFillRect(renderer, &rect);
		}
	}
}

void DrawText(SDL_Renderer *renderer, Canvas *font, Palette *palette, int x0, int y0, int pixelSize, char *text) {
	int i;
	int x = x0, y = y0;
	for(i=0;i<strlen(text);i++) {
		DrawChar(renderer,font,palette,x,y,pixelSize,text[i]);
		x+=font->w*pixelSize;
		if(x+font->w>=SCREEN_WIDTH) {
			x=0;
			y+=font->h*pixelSize;
		}
	}
}


int main(int argc,char *argv[]) {

	if(argc!=2) {
		printf("Syntax: %s [filename.cvs]\n",argv[0]);
		return EXIT_FAILURE;
	}

	SDL_Init(SDL_INIT_VIDEO);

    window = SDL_CreateWindow(GAME_TITLE,
             SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
             SCREEN_WIDTH, SCREEN_HEIGHT,
             SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

    renderer = SDL_CreateRenderer(window, -1,
               SDL_RENDERER_ACCELERATED |
               SDL_RENDERER_TARGETTEXTURE);

	palette = Palette_New(colors, ncolors, 0, SCREEN_HEIGHT - 32, 32 * ncolors, 32, 0, 32);

	mouse = Mouse_New("images/mouse.bmp", SCREEN_WIDTH/2, SCREEN_HEIGHT/2, colors[15], 0, 0);
	SDL_SetCursor(mouse->cursor);
	SDL_WarpMouseInWindow(window, mouse->x, mouse->y);

	canvas=Canvas_LoadCVS(argv[1], palette);
	history = History_New(canvas->w * canvas->h * canvas->nframe, 100);

	target=Target_New(palette, canvas, canvas->w / 2, canvas->h / 2);

	myWindow = MyWindow_New(palette, canvas, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT - 32, 16);


	font=Canvas_LoadCVS("cvs/font-00.cvs", palette);

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
				case SDLK_g:
					canvas->gridShow = !canvas->gridShow;
					break;
                case SDLK_z: {
                    int oldPixelSize = canvas->pixelSize;
                    int targetCellX = target->cellX;
                    int targetCellY = target->cellY;
                    int step, targetWorldX, targetWorldY;

                    if(event.key.keysym.mod & KMOD_SHIFT) {
                        if(canvas->pixelSize > 1) canvas->pixelSize--;
                    } else {
                        if(canvas->pixelSize < 32) canvas->pixelSize++;
                    }

                    if(oldPixelSize != canvas->pixelSize) {
                        step = canvas->gridShow ? canvas->pixelSize + 1 : canvas->pixelSize;
                        targetWorldX = targetCellX * step + (canvas->pixelSize / 2);
                        targetWorldY = targetCellY * step + (canvas->pixelSize / 2);

                        myWindow->hscroll->scrollPosition = targetWorldX - (myWindow->viewPortW / 2);
                        myWindow->vscroll->scrollPosition = targetWorldY - (myWindow->viewPortH / 2);
                    }
                    break;
                }
                case SDLK_u:
                    if(event.key.keysym.mod & KMOD_SHIFT) {
                        History_Redo(history, canvas->pixels);
                    } else {
                        History_Undo(history, canvas->pixels);
                    }
                    break;
				case SDLK_y:
					if(event.key.keysym.mod & KMOD_CTRL) {
						History_Redo(history, canvas->pixels);
					}
					break;
                case SDLK_c: {
                    int step = canvas->gridShow ? canvas->pixelSize + 1 : canvas->pixelSize;
                    int targetWorldX = target->cellX * step + (canvas->pixelSize / 2);
                    int targetWorldY = target->cellY * step + (canvas->pixelSize / 2);
                    myWindow->hscroll->scrollPosition = targetWorldX - (myWindow->viewPortW / 2);
                    myWindow->vscroll->scrollPosition = targetWorldY - (myWindow->viewPortH / 2);
                    break;
                }
				case SDLK_l:
					History_Push(history, canvas->pixels);
					Canvas_MouseToCell(canvas,target->x,target->y,&tx,&ty);
					Canvas_MouseToCell(canvas,mouse->x,mouse->y,&cx,&cy);
					Canvas_DrawLine(canvas, tx, ty, cx, cy, palette->currentColor);
					if(!(event.key.keysym.mod & KMOD_SHIFT)) {
						target->cellX=cx;
						target->cellY=cy;
					}
					break;
				case SDLK_r:
					History_Push(history, canvas->pixels);
					Canvas_MouseToCell(canvas,target->x,target->y,&tx,&ty);
					Canvas_MouseToCell(canvas,mouse->x,mouse->y,&cx,&cy);
					if(event.key.keysym.mod & KMOD_SHIFT) {
						Canvas_FillRect(canvas, tx, ty, cx, cy, palette->currentColor);
					} else {
						Canvas_DrawRect(canvas, tx, ty, cx, cy, palette->currentColor);
					}
					break;
				case SDLK_o:
					History_Push(history, canvas->pixels);
					Canvas_MouseToCell(canvas,target->x,target->y,&tx,&ty);
					Canvas_MouseToCell(canvas,mouse->x,mouse->y,&cx,&cy);
					if(event.key.keysym.mod & KMOD_SHIFT) {
						Canvas_FillOval(canvas, tx, ty, cx, cy, palette->currentColor);
					} else {
						Canvas_DrawOval(canvas, tx, ty, cx, cy, palette->currentColor);
					}
					break;
				case SDLK_f:
					History_Push(history, canvas->pixels);
					Canvas_MouseToCell(canvas,mouse->x,mouse->y,&cx,&cy);
					oldColor = Canvas_ReadPoint(canvas,cx,cy);
					newColor = palette->currentColor;
					Canvas_FloodFill(canvas, cx, cy, newColor, oldColor);
					break;
				case SDLK_s:
					if(event.key.keysym.mod & KMOD_SHIFT) {
					    MyWindow_Free(myWindow);
					    Target_Free(target);
					    Canvas_Free(canvas);
					    History_Free(history);

                        canvas = Canvas_LoadCVS(argv[1], palette);
                        history = History_New(canvas->w * canvas->h * canvas->nframe, 100);
                    	target=Target_New(palette, canvas, canvas->w / 2, canvas->h / 2);
                    	myWindow = MyWindow_New(palette, canvas, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT - 32, 16);
    				} else {
    					Canvas_SaveCVS(canvas,argv[1]);
    				}
					break;
				case SDLK_n:
					if(event.key.keysym.mod & KMOD_SHIFT) {
						canvas->frame--;
						if(canvas->frame<0) canvas->frame=canvas->nframe-1;
						break;
					} else {
						canvas->frame++;
						if(canvas->frame>=canvas->nframe) canvas->frame=0;
					}
					break;
				default:
					break;
				}
				break;
			case SDL_MOUSEBUTTONDOWN:
				if(overPalette || palette->scrubbing || overScrollBar || draggingScrollBar) continue;

				if(!isDrawing && event.button.button == SDL_BUTTON_LEFT) {
					Canvas_MouseToCell(canvas, event.button.x, event.button.y, &cx, &cy);
					if(cx >= 0 && cx < canvas->w && cy >= 0 && cy < canvas->h) {
						History_Push(history, canvas->pixels);
						isDrawing = true;
						dx = cx;
						dy = cy;
						Canvas_DrawPoint(canvas, dx, dy, palette->currentColor);
					}
				}
				break;
			case SDL_MOUSEBUTTONUP:
				isDrawing = false;
				break;
			case SDL_MOUSEMOTION:
				if(overPalette || palette->scrubbing || overScrollBar || draggingScrollBar) continue;
				if(isDrawing) {
					Canvas_MouseToCell(canvas, event.motion.x, event.motion.y, &cx, &cy);
					if(cx >= 0 && cx < canvas->w && cy >= 0 && cy < canvas->h) {
						Canvas_DrawLine(canvas, dx, dy, cx, cy, palette->currentColor);
						dx = cx;
						dy = cy;
					}
				}
				break;
            case SDL_MOUSEWHEEL:
                if(overPalette || palette->scrubbing || overScrollBar || draggingScrollBar) continue;
                {
                    /* C89 requires declarations at the start of the block */
                    int oldPixelSize = canvas->pixelSize;
                    int targetCellX = target->cellX;
                    int targetCellY = target->cellY;
                    int step, targetWorldX, targetWorldY;

                    if(event.wheel.direction == SDL_MOUSEWHEEL_FLIPPED) {
                        if(event.wheel.y > 0) event.wheel.y *= -1;
                    }

                    if(canvas->pixelSize > 1 && event.wheel.y < 0) {
                        canvas->pixelSize--;
                    }
                    else if(canvas->pixelSize < 32 && event.wheel.y > 0) {
                        canvas->pixelSize++;
                    }

                    /* If the zoom level changed, recalculate scroll positions to center the target */
                    if (oldPixelSize != canvas->pixelSize) {
                        step = canvas->gridShow ? canvas->pixelSize + 1 : canvas->pixelSize;

                        /* Calculate the target's center in world pixels at the new scale */
                        targetWorldX = targetCellX * step + (canvas->pixelSize / 2);
                        targetWorldY = targetCellY * step + (canvas->pixelSize / 2);

                        /* Adjust scrollbars so the target world position is at the viewport center */
                        myWindow->hscroll->scrollPosition = targetWorldX - (myWindow->viewPortW / 2);
                        myWindow->vscroll->scrollPosition = targetWorldY - (myWindow->viewPortH / 2);

                        /* ScrollBar_Update (via MyWindow_Update) will clamp these values in the next frame */
                    }
                }
                break;
	        default:
	        	break;
            }
            
            Mouse_EventHandle(mouse,event);

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

		{
			char msg[256];
			sprintf(msg,"%3d",canvas->frame);
			DrawText(renderer,font,palette,SCREEN_WIDTH-3*font->w*2,SCREEN_HEIGHT-font->h*2,2,msg);
		}

	    SDL_RenderPresent(renderer);

	    SDL_Delay(10);
    }

	MyWindow_Free(myWindow);
	Palette_Free(palette);
	Canvas_Free(canvas);
	History_Free(history);
	Target_Free(target);
	Mouse_Free(mouse);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();

	return EXIT_SUCCESS;
}


