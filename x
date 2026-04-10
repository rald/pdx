/* file: canvas.c */

#include "canvas.h"

typedef struct {
  int x,y;
} Point;

#define STACK_MAX 1024
Point stack[STACK_MAX];
int sp=STACK_MAX;

static void push(int x,int y) {
  sp--;
  stack[sp].x=x;
  stack[sp].y=y;
}

static void pop(int *x,int *y) {
  *x=stack[sp].x;
  *y=stack[sp].y;
  sp++;
}

static bool isStackEmpty() {
  return sp==STACK_MAX;
}

static void scanForSeeds(Canvas *canvas, int lx, int rx, int y, byte old_color, byte new_color) {  
  bool added = false;
  int x;
  
  if (y < 0 || y >= SCREEN_HEIGHT) return;
  
  for (x = lx; x <= rx; x++) {
    if (Canvas_ReadPoint(canvas, x, y) == old_color) {
      if (!added) {
        push(x, y);
        added = true;
      }
    } else {
      added = false;
    }
  }
}

void Canvas_FloodFill(Canvas *canvas, int x, int y, byte new_color, byte old_color) {
  int i;
  int lx,rx;
  if (old_color == new_color) return;
  
  push(x, y);
  
  while (!isStackEmpty()) {
    pop(&x, &y);
  
    lx = x;
    while (lx > 0 && Canvas_ReadPoint(canvas, lx - 1, y) == old_color) {
      Canvas_DrawPoint(canvas, lx - 1, y, new_color);
      lx--;
    }
    
    rx = x;
    while (rx < SCREEN_WIDTH - 1 && Canvas_ReadPoint(canvas, rx + 1, y) == old_color) {
      Canvas_DrawPoint(canvas, rx + 1, y, new_color);
      rx++;
    }
    
    for (i = lx; i <= rx; i++) {
      Canvas_DrawPoint(canvas, i, y, new_color);
    }
    
    scanForSeeds(canvas, lx, rx, y + 1, old_color, new_color);
    scanForSeeds(canvas, lx, rx, y - 1, old_color, new_color);
  }
}

void Canvas_MouseToCell(Canvas *canvas, int mx, int my, int *cx, int *cy) {
    int step = canvas->gridShow ? canvas->pixelSize + 1 : canvas->pixelSize;
    int localX = mx - canvas->x;
    int localY = my - canvas->y;

    if(step <= 0) {
        *cx = -1;
        *cy = -1;
        return;
    }

    *cx = localX / step;
    *cy = localY / step;

    if(localX < 0) (*cx)--;
    if(localY < 0) (*cy)--;
}

Canvas *Canvas_New(
    Palette *palette,
    int x, int y, int w, int h,
    int nframe,
    int transparent,
    byte color,
    byte gridColor,
    bool gridShow,
    int pixelSize,
    int frame) {

	int i;

    Canvas *canvas = malloc(sizeof(*canvas));
    if(!canvas) return NULL;

    canvas->palette = palette;
    
    canvas->w = w;
    canvas->h = h;
    canvas->nframe = nframe;
    canvas->transparent = transparent;

   	canvas->delays=malloc(sizeof(*canvas->delays)*canvas->nframe);
	for(i = 0; i < canvas->nframe; i++) {
		canvas->delays[i]=10;
	}

    canvas->pixels = calloc(w * h * nframe, sizeof(*canvas->pixels));
    if(!canvas->pixels) {
        free(canvas);
        return NULL;
    }

    for(i = 0; i < w * h * nframe; i++) canvas->pixels[i] = color;

    canvas->pixelSize = pixelSize;
    canvas->x = x;
    canvas->y = y;
    canvas->frame = frame;
    canvas->gridColor = gridColor;
    canvas->gridShow = gridShow;

    return canvas;
}

void Canvas_Free(Canvas *canvas) {
	free(canvas->pixels);
	free(canvas->delays);
	free(canvas);
}

void Canvas_Draw(Canvas *canvas, SDL_Renderer *renderer, SDL_Rect viewport) {
    int i, j;
    int px = canvas->pixelSize;
    
    /* If grid is shown, each 'cell' takes up pixelSize + 1 for the line */
    int step = canvas->gridShow ? px + 1 : px;
    if(step <= 0) return;

    int base = canvas->frame * canvas->w * canvas->h;
    
    /* Calculate visible range */
    int x0 = (-canvas->x) / step;
    int y0 = (-canvas->y) / step;
    int x1 = (-canvas->x + viewport.w) / step + 1;
    int y1 = (-canvas->y + viewport.h) / step + 1;

    if(x0 < 0) x0 = 0;
    if(y0 < 0) y0 = 0;
    if(x1 > canvas->w) x1 = canvas->w;
    if(y1 > canvas->h) y1 = canvas->h;

    SDL_RenderSetClipRect(renderer, &viewport);

    /* 1. Draw the Grid Lines (if enabled) */
    if (canvas->gridShow) {
        SDL_Color gc = canvas->palette->colors[canvas->gridColor];
        SDL_SetRenderDrawColor(renderer, gc.r, gc.g, gc.b, 255);

        /* Draw Vertical Lines */
        for (i = x0; i <= x1; i++) {
            int lineX = canvas->x + (i * step);
            SDL_RenderDrawLine(renderer, lineX, viewport.y, lineX, viewport.y + viewport.h);
        }

        /* Draw Horizontal Lines */
        for (j = y0; j <= y1; j++) {
            int lineY = canvas->y + (j * step);
            SDL_RenderDrawLine(renderer, viewport.x, lineY, viewport.x + viewport.w, lineY);
        }
    }

    /* 2. Draw the Pixels */
    SDL_Rect rect;
    for(j = y0; j < y1; j++) {
        int row = base + j * canvas->w;
        /* Offset the pixel by 1 if grid is showing to not overlap the top/left grid line */
        rect.y = canvas->y + j * step + (canvas->gridShow ? 1 : 0);
        rect.h = px;

        for(i = x0; i < x1; i++) {
            byte l = canvas->pixels[row + i];
            if(l == canvas->transparent) continue;

            rect.x = canvas->x + i * step + (canvas->gridShow ? 1 : 0);
            rect.w = px;

            SDL_Color c = canvas->palette->colors[l];
            SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, 255);
            SDL_RenderFillRect(renderer, &rect);
        }
    }

    SDL_RenderSetClipRect(renderer, NULL);
}


void Canvas_DrawPoint(Canvas *canvas, int x, int y, byte color) {
    if(x >= 0 && x < canvas->w && y >= 0 && y < canvas->h) {
        canvas->pixels[canvas->frame * canvas->w * canvas->h + y * canvas->w + x] = color;
    }
}

int Canvas_ReadPoint(Canvas *canvas, int x, int y) {
    if(x >= 0 && x < canvas->w && y >= 0 && y < canvas->h) {
        return canvas->pixels[canvas->frame * canvas->w * canvas->h + y * canvas->w + x];
    }
    return -1;
}

void Canvas_DrawLine(Canvas *canvas, int x0, int y0, int x1, int y1, byte color) {
    int dx = abs(x1 - x0);
    int sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0);
    int sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;

    while(true) {
        Canvas_DrawPoint(canvas, x0, y0, color);
        if(x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if(e2 >= dy) { err += dy; x0 += sx; }
        if(e2 <= dx) { err += dx; y0 += sy; }
    }
}

void Canvas_DrawRect(Canvas *canvas,int x0,int y0,int x1,int y1,int c) {
  int t,i,j;

  if(x0>x1) { t=x0; x0=x1; x1=t; }
  if(y0>y1) { t=y0; y0=y1; y1=t; }

  for(i=x0;i<=x1;i++) {
    Canvas_DrawPoint(canvas,i,y0,c);
    Canvas_DrawPoint(canvas,i,y1,c);
  }

  for(j=y0;j<=y1;j++) {
    Canvas_DrawPoint(canvas,x0,j,c);
    Canvas_DrawPoint(canvas,x1,j,c);
  }
}

void Canvas_FillRect(Canvas *canvas,int x0,int y0,int x1,int y1,int c) {
  int t,i,j;

  if(x0>x1) { t=x0; x0=x1; x1=t; }
  if(y0>y1) { t=y0; y0=y1; y1=t; }

  for(j=y0;j<=y1;j++) {
    for(i=x0;i<=x1;i++) {
      Canvas_DrawPoint(canvas,i,j,c);
    }
  }

}

static void plotPoints(Canvas *canvas, int xc,int yc,int x,int y,int c) {
  Canvas_DrawPoint(canvas, xc + x, yc + y, c);
  Canvas_DrawPoint(canvas, xc - x, yc + y, c);
  Canvas_DrawPoint(canvas, xc + x, yc - y, c);
  Canvas_DrawPoint(canvas, xc - x, yc - y, c);
}

void Canvas_DrawOval(Canvas *canvas,int x1, int y1, int x2, int y2,int c) {
    int xc = (x1 + x2) / 2;
    int yc = (y1 + y2) / 2;
    long a = abs(x2 - x1) / 2;
    long b = abs(y2 - y1) / 2;

    long a2 = a * a;
    long b2 = b * b;
    long x = 0;
    long y = b;
    long d1 = b2 - (a2 * b) + (0.25 * a2);
    long d2;
    long dx = 2 * b2 * x;
    long dy = 2 * a2 * y;

    while (dx < dy) {
        plotPoints(canvas, xc, yc, x, y, c);
        if (d1 < 0) {
            x++;
            dx += 2 * b2;
            d1 += dx + b2;
        } else {
            x++;
            y--;
            dx += 2 * b2;
            dy -= 2 * a2;
            d1 += dx - dy + b2;
        }
    }

    d2 = b2 * (x + 0.5) * (x + 0.5) + a2 * (y - 1) * (y - 1) - a2 * b2;
    while (y >= 0) {
        plotPoints(canvas, xc, yc, x, y, c);
        if (d2 > 0) {
            y--;
            dy -= 2 * a2;
            d2 += a2 - dy;
        } else {
            y--;
            x++;
            dx += 2 * b2;
            dy -= 2 * a2;
            d2 += dx - dy + a2;
        }
    }
}

void Canvas_FillOval(Canvas *canvas,int x0,int y0,int x1,int y1,int c) {
  int i,t,xc,yc,rx,ry;
  long x,y;
  long rx2,ry2;
  long tworx2,twory2;
  long p;

  if(x0>x1) { t=x0; x0=x1; x1=t; }
  if(y0>y1) { t=y0; y0=y1; y1=t; }

  xc=(x0+x1)/2;
  yc=(y0+y1)/2;
  rx=(x1-x0)/2;
  ry=(y1-y0)/2;

  x=0; y=ry;
  rx2=rx*rx;
  ry2=ry*ry;
  tworx2=2*rx2;
  twory2=2*ry2;

  p=ry2-(rx2*ry)+(0.25*rx2);

  while(twory2 * x <= tworx2 * y) {
    for(i=xc-x;i<=xc+x;i++) {
      Canvas_DrawPoint(canvas,i,yc+y,c);
      Canvas_DrawPoint(canvas,i,yc-y,c);
    }

    x++;
    if(p<0) {
      p+=twory2*x+ry2;
    } else {
      y--;
      p+=twory2*x-tworx2*y+ry2;
    }
  }

  p=ry2*(x+0.5)*(x+0.5)+rx2*(y-1)*(y-1)-rx2*ry2;

  while(y>=0) {
    for(i=xc-x;i<=xc+x;i++) {
      Canvas_DrawPoint(canvas,i,yc+y,c);
      Canvas_DrawPoint(canvas,i,yc-y,c);
    }
  
    y--;
    if(p>0) {
      p+=rx2-tworx2*y;
    } else {
      x++;
      p+=twory2*x-tworx2*y+rx2;
    }
  }
}

Canvas *Canvas_LoadCVS(char *filename, Palette *palette) {
	int i,j,k,l;
	int c;
	char *hex="0123456789ABCDEF";
	Canvas *canvas=malloc(sizeof(*canvas));
	FILE *fp = NULL;

	if(canvas) {
		canvas->palette = palette;
		canvas->x = 0;
		canvas->y = 0;
		canvas->gridColor = 10;
		canvas->gridShow = false;
		canvas->pixelSize = 1;
		canvas->frame = 0;

		fp=fopen(filename,"rb");
		if(!fp) {
			fprintf(stderr,"Error opening file %s: %s\n",filename,strerror(errno));
			exit(-1);
		}

		fscanf(fp,"%d,%d,%d,%d",&canvas->w,&canvas->h,&canvas->nframe,&canvas->transparent);

       	canvas->delays=malloc(sizeof(*canvas->delays)*canvas->nframe);
	    for(i = 0; i < canvas->nframe; i++) {
		    if(i!=0) fscanf(fp,",");
		    fscanf(fp,"%d",&canvas->delays[i]);
	    }

		canvas->pixels = malloc((canvas->w * canvas->h * canvas->nframe) * sizeof(*canvas->pixels));
		for(i = 0; i < canvas->w * canvas->h * canvas->nframe; i++) canvas->pixels[i]=12;

		i=0; l=1;
		while((c=fgetc(fp))!=EOF) {
			if(c==' ' || c=='\t') continue;
			if(c=='\n') { l++; continue; }
			j=-1;
			for(k=0;k<16;k++) {
				if(c==hex[k]) {
					j=k;
					break;
				}
			}
			if(j==-1) {
				fprintf(stderr,"File %s: Line %d: invalid character.\n",filename,l);
				exit(-1);
			}
			canvas->pixels[i++]=j;
		}
		fclose(fp);

	}

	return canvas;
}

Canvas *Canvas_SaveCVS(Canvas *canvas, char *filename) {
	int f,i,j,k;
	int c;
	char *hex="0123456789ABCDEF";

	FILE *fp=fopen(filename,"wb");

	fprintf(fp,"%d,%d,%d,%d\n",canvas->w,canvas->h,canvas->nframe,canvas->transparent);

	for(f = 0; f < canvas->nframe; f++) {
		if(f!=0) fprintf(fp,",");
		fprintf(fp,"%d",canvas->delays[f]);
	}
	fprintf(fp,"\n\n");

	for(f=0;f<canvas->nframe;f++) {
		for(j=0;j<canvas->h;j++) {
			for(i=0;i<canvas->w;i++) {
				k=hex[canvas->pixels[f*canvas->w*canvas->h+j*canvas->w+i]];
				fputc(k,fp);
			}
			fputc('\n',fp);
		}
		fputc('\n',fp);
	}
	fputc('\n',fp);

	fclose(fp);
}

void Canvas_AddFrameBefore(Canvas *canvas) {
	int frameSize = canvas->w * canvas->h;
	int oldNframe = canvas->nframe;
	int currentFrame = canvas->frame;
	int i;
	byte *newPixels;
	int *newDelays;

	newPixels = realloc(canvas->pixels, canvas->w * canvas->h * (oldNframe + 1));
	if (!newPixels) return;
	canvas->pixels = newPixels;

	newDelays = realloc(canvas->delays, sizeof(int) * (oldNframe + 1));
	if (!newDelays) return;
	canvas->delays = newDelays;

	canvas->nframe++;

	/* Move frames from currentFrame onwards one step to the right */
	memmove(canvas->pixels + (currentFrame + 1) * frameSize,
			canvas->pixels + currentFrame * frameSize,
			(oldNframe - currentFrame) * frameSize);

	memmove(canvas->delays + currentFrame + 1,
			canvas->delays + currentFrame,
			sizeof(int) * (oldNframe - currentFrame));

	/* Initialize new frame */
	for (i = 0; i < frameSize; i++) {
		canvas->pixels[currentFrame * frameSize + i] = canvas->transparent;
	}
	canvas->delays[currentFrame] = 10;
}

void Canvas_AddFrameAfter(Canvas *canvas) {
	int frameSize = canvas->w * canvas->h;
	int oldNframe = canvas->nframe;
	int nextFrame = canvas->frame + 1;
	int i;
	byte *newPixels;
	int *newDelays;

	newPixels = realloc(canvas->pixels, canvas->w * canvas->h * (oldNframe + 1));
	if (!newPixels) return;
	canvas->pixels = newPixels;

	newDelays = realloc(canvas->delays, sizeof(int) * (oldNframe + 1));
	if (!newDelays) return;
	canvas->delays = newDelays;

	canvas->nframe++;

	if (nextFrame < canvas->nframe - 1) {
		memmove(canvas->pixels + (nextFrame + 1) * frameSize,
				canvas->pixels + nextFrame * frameSize,
				(oldNframe - nextFrame) * frameSize);

		memmove(canvas->delays + nextFrame + 1,
				canvas->delays + nextFrame,
				sizeof(int) * (oldNframe - nextFrame));
	}

	/* Initialize new frame */
	for (i = 0; i < frameSize; i++) {
		canvas->pixels[nextFrame * frameSize + i] = canvas->transparent;
	}
	canvas->delays[nextFrame] = 10;
	canvas->frame = nextFrame;
}

void Canvas_RemoveFrame(Canvas *canvas) {
	int frameSize = canvas->w * canvas->h;
	int currentFrame = canvas->frame;
	byte *newPixels;
	int *newDelays;

	if (canvas->nframe <= 1) return;

	if (currentFrame < canvas->nframe - 1) {
		memmove(canvas->pixels + currentFrame * frameSize,
				canvas->pixels + (currentFrame + 1) * frameSize,
				(canvas->nframe - currentFrame - 1) * frameSize);

		memmove(canvas->delays + currentFrame,
				canvas->delays + currentFrame + 1,
				sizeof(int) * (canvas->nframe - currentFrame - 1));
	}

	canvas->nframe--;
	newPixels = realloc(canvas->pixels, canvas->w * canvas->h * canvas->nframe);
	if (newPixels) canvas->pixels = newPixels;

	newDelays = realloc(canvas->delays, sizeof(int) * canvas->nframe);
	if (newDelays) canvas->delays = newDelays;

	if (canvas->frame >= canvas->nframe) {
		canvas->frame = canvas->nframe - 1;
	}
}
/* file: canvas.h */

#ifndef CANVAS_H
#define CANVAS_H

#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>

#include "common.h"
#include "types.h"
#include "palette.h"

typedef struct Canvas {
	Palette *palette;
	int x,y,w,h;
	int nframe;
	int transparent;
	byte *pixels;
	byte gridColor;
	bool gridShow;
	int pixelSize;
	int frame;
	int *delays;
} Canvas;

Canvas *Canvas_New(
    Palette *palette,
    int x, int y, int w, int h,
    int nframe,
    int transparent,
    byte color,
    byte gridColor,
    bool gridShow,
    int pixelSize,
    int frame
);

void Canvas_Free(Canvas *canvas);

Canvas *Canvas_LoadCVS(char *filename, Palette *palette);

void Canvas_Draw(Canvas *canvas, SDL_Renderer *renderer, SDL_Rect viewport);

void Canvas_DrawPoint(Canvas *canvas, int x, int y, byte color);
int Canvas_ReadPoint(Canvas *canvas, int x, int y);
void Canvas_DrawLine(Canvas *canvas, int x0, int y0, int x1, int y1, byte color);

void Canvas_MouseToCell(Canvas *canvas, int mx, int my, int *cx, int *cy);

void Canvas_FloodFill(Canvas *canvas, int x, int y, byte new_color, byte old_color);

void Canvas_DrawRect(Canvas *canvas,int x0,int y0,int x1,int y1,int c);
void Canvas_FillRect(Canvas *canvas,int x0,int y0,int x1,int y1,int c);
void Canvas_DrawOval(Canvas *canvas,int x1, int y1, int x2, int y2,int c);
void Canvas_FillOval(Canvas *canvas,int x0,int y0,int x1,int y1,int c);

void Canvas_AddFrameBefore(Canvas *canvas);
void Canvas_AddFrameAfter(Canvas *canvas);
void Canvas_RemoveFrame(Canvas *canvas);

#endif


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
	history = History_New(canvas->w, canvas->h, 100);

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
                        History_Redo(history, canvas);
                    } else {
                        History_Undo(history, canvas);
                    }
                    break;
				case SDLK_y:
					if(event.key.keysym.mod & KMOD_CTRL) {
						History_Redo(history, canvas);
					}
					break;
                case SDLK_x: {
                    int step = canvas->gridShow ? canvas->pixelSize + 1 : canvas->pixelSize;
                    int targetWorldX = target->cellX * step + (canvas->pixelSize / 2);
                    int targetWorldY = target->cellY * step + (canvas->pixelSize / 2);
                    myWindow->hscroll->scrollPosition = targetWorldX - (myWindow->viewPortW / 2);
                    myWindow->vscroll->scrollPosition = targetWorldY - (myWindow->viewPortH / 2);
                    break;
                }
				case SDLK_l:
					History_Push(history, canvas);
					Canvas_MouseToCell(canvas,target->x,target->y,&tx,&ty);
					Canvas_MouseToCell(canvas,mouse->x,mouse->y,&cx,&cy);
					Canvas_DrawLine(canvas, tx, ty, cx, cy, palette->currentColor);
					if(!(event.key.keysym.mod & KMOD_SHIFT)) {
						target->cellX=cx;
						target->cellY=cy;
					}
					break;
				case SDLK_a:
					History_Push(history, canvas);
					if(event.key.keysym.mod & KMOD_SHIFT) {
						Canvas_AddFrameAfter(canvas);
					} else {
						Canvas_AddFrameBefore(canvas);
					}
					break;
				case SDLK_d:
					History_Push(history, canvas);
					Canvas_RemoveFrame(canvas);
					break;
				case SDLK_r:
					History_Push(history, canvas);
					Canvas_MouseToCell(canvas,target->x,target->y,&tx,&ty);
					Canvas_MouseToCell(canvas,mouse->x,mouse->y,&cx,&cy);
					if(event.key.keysym.mod & KMOD_SHIFT) {
						Canvas_FillRect(canvas, tx, ty, cx, cy, palette->currentColor);
					} else {
						Canvas_DrawRect(canvas, tx, ty, cx, cy, palette->currentColor);
					}
					break;
				case SDLK_o:
					History_Push(history, canvas);
					Canvas_MouseToCell(canvas,target->x,target->y,&tx,&ty);
					Canvas_MouseToCell(canvas,mouse->x,mouse->y,&cx,&cy);
					if(event.key.keysym.mod & KMOD_SHIFT) {
						Canvas_FillOval(canvas, tx, ty, cx, cy, palette->currentColor);
					} else {
						Canvas_DrawOval(canvas, tx, ty, cx, cy, palette->currentColor);
					}
					break;
				case SDLK_f:
					History_Push(history, canvas);
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
                        history = History_New(canvas->w, canvas->h, 100);
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
				case SDLK_c:
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
						History_Push(history, canvas);
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


