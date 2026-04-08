# file: Makefile

CC = gcc
EXE = game

CFLAGS = -lSDL2 -lgifenc -lgifdec -g -I. -I lecram/gifenc -I lecram/gifdec
LDFLAGS = -L. -L lecram/gifenc -L lecram/gifdec
LDLIBS = -std=c89 -lm -g

OBJS = main.o utils.o canvas.o mouse.o target.o palette.o button.o scrollbar.o MyWindow.o

all: libgifenc.a libgifdec.a game

libgifenc.a: lecram/gifenc/gifenc.c lecram/gifenc/gifenc.h
	$(CC) -c lecram/gifenc/gifenc.c -o lecram/gifenc/gifenc.o
	ar rcs lecram/gifenc/libgifenc.a lecram/gifenc/gifenc.o

libgifdec.a: lecram/gifdec/gifdec.c lecram/gifdec/gifdec.h
	$(CC) -c lecram/gifdec/gifdec.c -o lecram/gifdec/gifdec.o
	ar rcs lecram/gifdec/libgifdec.a lecram/gifdec/gifdec.o

game: $(OBJS)
	$(CC) -o $(EXE) $(OBJS) $(CFLAGS) $(LDFLAGS) $(LDLIBS)

%.o: %.c
	$(CC) -c $< -o $@ $(LDFLAGS) $(CFLAGS) $(LDLIBS)

clean:
	rm game
	rm $(OBJS)
	rm lecram/gifenc/libgifenc.a lecram/gifenc/gifenc.o
	rm lecram/gifdec/libgifdec.a lecram/gifdec/gifdec.o
/* MyWindow.c */

#include <stdlib.h>
#include "MyWindow.h"

MyWindow *MyWindow_New(
		Palette *palette, 
		Canvas *canvas, 
		int x, int y, int w, int h, 
		int buttonSize) {

    MyWindow *myWindow = malloc(sizeof(*myWindow));
    if(!myWindow) return NULL;

    myWindow->x = x;
    myWindow->y = y;
    myWindow->w = w;
    myWindow->h = h;
    myWindow->canvas = canvas;

	myWindow->buttonSize = buttonSize;

    myWindow->viewPortW = w - buttonSize;
    myWindow->viewPortH = h - buttonSize;

    myWindow->vscroll = ScrollBar_New(
        palette, canvas,
        SCROLLBAR_ORIENTATION_VERTICAL,
        x + w - buttonSize, y, buttonSize, h - buttonSize, 
        myWindow->viewPortW, myWindow->viewPortH,
        buttonSize
    );

    myWindow->hscroll = ScrollBar_New(
        palette, canvas,
        SCROLLBAR_ORIENTATION_HORIZONTAL,
        x, y + h - buttonSize, w - buttonSize, buttonSize, 
        myWindow->viewPortW, myWindow->viewPortH,
        buttonSize
    );

    myWindow->canvas->x = x;
    myWindow->canvas->y = y;

    return myWindow;
}

void MyWindow_Free(MyWindow *myWindow) {
	ScrollBar_Free(myWindow->vscroll);
	ScrollBar_Free(myWindow->hscroll);
	free(myWindow);
}

void MyWindow_Update(MyWindow *myWindow, Mouse *mouse) {
    ScrollBar_Update(myWindow->vscroll, mouse);
    ScrollBar_Update(myWindow->hscroll, mouse);
}

void MyWindow_Draw(MyWindow *myWindow, SDL_Renderer *renderer) {
    SDL_Rect viewPort = { myWindow->x, myWindow->y, myWindow->viewPortW, myWindow->viewPortH };
    Canvas_Draw(myWindow->canvas, renderer, viewPort);
    ScrollBar_Draw(myWindow->vscroll, renderer);
    ScrollBar_Draw(myWindow->hscroll, renderer);
}


/* file: button.c */

#include "button.h"

Button *Button_New(int x, int y, int w, int h, Palette *palette) {
	Button *button = malloc(sizeof(*button));
	if(button) {
		button->state = BUTTON_STATE_UP;
		button->x = x;
		button->y = y;
		button->w = w;
		button->h = h;
		button->palette = palette;
	}
	return button;
}

void Button_Free(Button *button) {
	free(button);
}

void Button_Draw(Button *button, SDL_Renderer *renderer) {
	SDL_Rect rect = {button->x, button->y, button->w, button->h};
	switch(button->state) {
	case BUTTON_STATE_UP:
		SDL_SetRenderDrawColor(
			renderer,
			button->palette->colors[14].r,
			button->palette->colors[14].g,
			button->palette->colors[14].b,
			255);
		SDL_RenderFillRect(renderer, &rect);
		break;
	case BUTTON_STATE_OVER:
		SDL_SetRenderDrawColor(
			renderer,
			button->palette->colors[13].r,
			button->palette->colors[13].g,
			button->palette->colors[13].b,
			255);
		SDL_RenderFillRect(renderer, &rect);
		break;
	case BUTTON_STATE_DOWN: 
		SDL_SetRenderDrawColor(
			renderer,
			button->palette->colors[14].r,
			button->palette->colors[14].g,
			button->palette->colors[14].b,
			255);
		SDL_RenderFillRect(renderer, &rect);
		break;
	default: break;
	}

/*
	SDL_SetRenderDrawColor(
		renderer,
		button->palette->colors[12].r,
		button->palette->colors[12].g,
		button->palette->colors[12].b,
		255);

	SDL_RenderDrawRect(renderer, &rect);
*/

}

bool Button_Update(Button *button, Mouse *mouse) {
	SDL_Rect rect = {button->x, button->y, button->w, button->h};
	if(inrect(mouse->x,mouse->y,button->x,button->y,button->w,button->h)) {
		if(mouse->state & SDL_BUTTON_LMASK) {
			button->state=BUTTON_STATE_DOWN;
			return true;
		} else {
			button->state=BUTTON_STATE_OVER;
		}		
	} else {
		button->state=BUTTON_STATE_UP;	
	}		
	return false;
}


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
	free(canvas);
}

void Canvas_Draw(Canvas *canvas, SDL_Renderer *renderer, SDL_Rect viewport) {

	int i,j;

    int px = canvas->pixelSize;
    int step = canvas->gridShow ? px + 1 : px;
    if(step <= 0) return;

    int base = canvas->frame * canvas->w * canvas->h;
    int x0 = (-canvas->x) / step;
    int y0 = (-canvas->y) / step;
    int x1 = (-canvas->x + viewport.w) / step + 1;
    int y1 = (-canvas->y + viewport.h) / step + 1;

    if(x0 < 0) x0 = 0;
    if(y0 < 0) y0 = 0;
    if(x1 > canvas->w) x1 = canvas->w;
    if(y1 > canvas->h) y1 = canvas->h;

    SDL_RenderSetClipRect(renderer, &viewport);

    SDL_Rect rect;
    for(j = y0; j < y1; j++) {
        int row = base + j * canvas->w;
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
		canvas->gridColor = 6;
		canvas->gridShow = false;
		canvas->pixelSize = 1;
		canvas->frame = 0;

		fp=fopen(filename,"rb");
		if(!fp) {
			fprintf(stderr,"Error opening file %s: %s\n",filename,strerror(errno));
			exit(-1);
		}
		
		fscanf(fp,"%d,%d,%d,%d",&canvas->w,&canvas->h,&canvas->nframe,&canvas->transparent);

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
				fprintf(stderr,"Line %d: invalid character.\n",l);
				exit(-1);
			}
			canvas->pixels[i++]=j;
		}
		fclose(fp);

	}

	return canvas;
}

Canvas *Canvas_SaveCVS(Canvas *canvas, char *filename) {
	int i,j,k;
	int c;
	char *hex="0123456789ABCDEF";
	FILE *fp=fopen(filename,"wb");
	fprintf(fp,"%d,%d,%d,%d\n\n",canvas->w,canvas->h,canvas->nframe,canvas->transparent);
	for(j=0;j<canvas->h;j++) {
		for(i=0;i<canvas->w;i++) {
			k=hex[canvas->pixels[j*canvas->w+i]];
			fputc(k,fp);				
		}
		fputc('\n',fp);
	}
	fputc('\n',fp);
	fclose(fp);
}


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

bool isDrawing = false;
int dx = 0, dy = 0;
int cx = 0, cy = 0;
int tx = 0, ty = 0;
byte oldColor,newColor;



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
	
	target=Target_New(palette, canvas, canvas->w / 2, canvas->h / 2);

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
				case SDLK_g:
					canvas->gridShow = !canvas->gridShow;
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
					Canvas_MouseToCell(canvas,target->x,target->y,&tx,&ty);
					Canvas_MouseToCell(canvas,mouse->x,mouse->y,&cx,&cy);
					Canvas_DrawLine(canvas, tx, ty, cx, cy, palette->currentColor);
					if(!(event.key.keysym.mod & KMOD_SHIFT)) {
						target->cellX=cx;
						target->cellY=cy;					
					}
					break;
				case SDLK_r:
					Canvas_MouseToCell(canvas,target->x,target->y,&tx,&ty);
					Canvas_MouseToCell(canvas,mouse->x,mouse->y,&cx,&cy);
					if(event.key.keysym.mod & KMOD_SHIFT) {
						Canvas_FillRect(canvas, tx, ty, cx, cy, palette->currentColor);					
					} else {
						Canvas_DrawRect(canvas, tx, ty, cx, cy, palette->currentColor);
					}				
					break;
				case SDLK_o:
					Canvas_MouseToCell(canvas,target->x,target->y,&tx,&ty);
					Canvas_MouseToCell(canvas,mouse->x,mouse->y,&cx,&cy);
					if(event.key.keysym.mod & KMOD_SHIFT) {
						Canvas_FillOval(canvas, tx, ty, cx, cy, palette->currentColor);
					} else {
						Canvas_DrawOval(canvas, tx, ty, cx, cy, palette->currentColor);
					}
					break;
				case SDLK_f:
					Canvas_MouseToCell(canvas,mouse->x,mouse->y,&cx,&cy);
					oldColor = canvas->pixels[cy * canvas->w + cx];
					newColor = palette->currentColor;
					Canvas_FloodFill(canvas, cx, cy, newColor, oldColor);
					break;
				case SDLK_s:
					if(event.key.keysym.mod & KMOD_SHIFT) {
					    MyWindow_Free(myWindow);
					    Target_Free(target);
					    Canvas_Free(canvas);
					    					    
                        canvas = Canvas_LoadCVS(argv[1], palette);
                    	target=Target_New(palette, canvas, canvas->w / 2, canvas->h / 2);
                    	myWindow = MyWindow_New(palette, canvas, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT - 32, 16);
    				} else {
    					Canvas_SaveCVS(canvas,argv[1]);
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

	return EXIT_SUCCESS;
}


/* file: mouse.c */

#include <stdlib.h>
#include "mouse.h"

Mouse *Mouse_New(char *filename, int x, int y, SDL_Color transparent, int hotSpotX,int hotSpotY) {
	Mouse *mouse = malloc(sizeof(*mouse));
	SDL_Surface *surface = NULL;

	if(mouse) {
		mouse->x = x;
		mouse->y = y;
		mouse->state = 0;
		mouse->cursor = NULL;

		surface = SDL_LoadBMP(filename);
		if(surface) {
			SDL_SetColorKey(surface, SDL_TRUE, SDL_MapRGB(surface->format, transparent.r, transparent.g, transparent.b));
			mouse->cursor = SDL_CreateColorCursor(surface, hotSpotX, hotSpotY);
			SDL_FreeSurface(surface);
		}
	}

	return mouse;
}

void Mouse_Free(Mouse *mouse) {
	SDL_FreeCursor(mouse->cursor);
	free(mouse);
}

void Mouse_EventHandle(Mouse *mouse, SDL_Event event) {
	(void)mouse;
	(void)event;
}

void Mouse_Update(Mouse *mouse) {
	mouse->state = SDL_GetMouseState(&mouse->x, &mouse->y);
}/* file: palette.c */

#include "palette.h"

Palette *Palette_New(SDL_Color *colors, size_t ncolors, int x, int y, int w, int h, byte currentColor, size_t boxSize) {
	Palette *palette = malloc(sizeof(*palette));

	if(!palette) return NULL;

	palette->colors = colors;
	palette->ncolors = ncolors;
	palette->x = x;
	palette->y = y;
	palette->w = w;
	palette->h = h;
	palette->currentColor = currentColor;
	palette->boxSize = boxSize;
	palette->scrubbing = false;

	return palette;
}

void Palette_Free(Palette *palette) {
	free(palette);
}

void Palette_Update(Palette *palette, Mouse *mouse) {
    bool overPalette = inrect(mouse->x, mouse->y, palette->x, palette->y, palette->w, palette->h);

    if(mouse->state & SDL_BUTTON_LMASK) {
        if(overPalette || palette->scrubbing) {
            palette->scrubbing = true;
            int i = (mouse->x - palette->x) / palette->boxSize;
            if(i >= 0 && i < (int)palette->ncolors) {
                palette->currentColor = (byte)i;
            }
        }
    } else {
        palette->scrubbing = false;
    }
}

void Palette_Draw(Palette *palette, SDL_Renderer *renderer) {

	int i;

	SDL_Rect clip = { palette->x, palette->y, palette->w, palette->h};
	SDL_RenderSetClipRect(renderer, &clip);

	for(i = 0; i < palette->ncolors; i++) {
		
		SDL_Rect rect = {i*32,SCREEN_HEIGHT-32,32,32};
		
		SDL_SetRenderDrawColor(renderer, palette->colors[i].r, palette->colors[i].g, palette->colors[i].b, 255);
		SDL_RenderFillRect(renderer, &rect);

		if(i == palette->currentColor) {
			SDL_SetRenderDrawColor(renderer, palette->colors[0].r, palette->colors[0].g, palette->colors[0].b, 255);
			SDL_RenderFillRect(renderer, &rect);

			rect.x+=4;
			rect.y+=4;
			rect.w-=8;
			rect.h-=8;

			SDL_SetRenderDrawColor(renderer, palette->colors[i].r, palette->colors[i].g, palette->colors[i].b, 255);
			SDL_RenderFillRect(renderer, &rect);

			SDL_SetRenderDrawColor(renderer, palette->colors[12].r, palette->colors[12].g, palette->colors[12].b, 255);
			SDL_RenderDrawRect(renderer, &rect);
		}
	}

	SDL_Rect rect = {palette->x, palette->y, palette->w, palette->h};
	SDL_SetRenderDrawColor(renderer, palette->colors[12].r, palette->colors[12].g, palette->colors[12].b, 255);
	SDL_RenderDrawRect(renderer, &rect);
	
	SDL_RenderSetClipRect(renderer, NULL);
}
/* file: scrollbar.c */

#include <stdlib.h>
#include "scrollbar.h"

static int ScrollBar_GetViewPortSize(ScrollBar *s) {
    return s->orientation == SCROLLBAR_ORIENTATION_VERTICAL ? s->viewPortH : s->viewPortW;
}

static int ScrollBar_GetTrackArea(ScrollBar *s) {
    return s->orientation == SCROLLBAR_ORIENTATION_VERTICAL
        ? s->h - s->buttonSize * 2
        : s->w - s->buttonSize * 2;
}

static int ScrollBar_GetContentSize(ScrollBar *s) {
    if(s->orientation == SCROLLBAR_ORIENTATION_VERTICAL) {
        return s->canvas->gridShow
            ? s->canvas->h * (s->canvas->pixelSize + 1) + 1
            : s->canvas->h * s->canvas->pixelSize;
    }
    return s->canvas->gridShow
        ? s->canvas->w * (s->canvas->pixelSize + 1) + 1
        : s->canvas->w * s->canvas->pixelSize;
}

static int ScrollBar_GetTrackStart(ScrollBar *s) {
    return s->orientation == SCROLLBAR_ORIENTATION_VERTICAL ? s->y + s->buttonSize : s->x + s->buttonSize;
}

static void ScrollBar_UpdateThumbRect(ScrollBar *s) {
    if(s->orientation == SCROLLBAR_ORIENTATION_VERTICAL) {
        s->rectThumb = (SDL_Rect){ s->x, s->y + s->buttonSize + s->thumbPosition, s->w, s->thumbSize };
    } else {
        s->rectThumb = (SDL_Rect){ s->x + s->buttonSize + s->thumbPosition, s->y, s->thumbSize, s->h };
    }
}

static void ScrollBar_ApplyThumb(ScrollBar *s) {
    int maxScroll = s->contentSize - s->viewPortSize;

    if(maxScroll <= 0) {
        s->scrollPosition = 0;
        s->thumbPosition = 0;
        s->contentScrollPosition = 0;
        s->thumbSize = s->trackArea;
        if(s->orientation == SCROLLBAR_ORIENTATION_VERTICAL) s->canvas->y = 0;
        else s->canvas->x = 0;
        return;
    }

    s->scrollPosition = clamp(s->scrollPosition, 0, maxScroll);

    s->thumbSize = (int)((double)s->viewPortSize / s->contentSize * s->trackArea);
    s->thumbSize = clamp(s->thumbSize, 1, s->trackArea);

    int span = s->trackArea - s->thumbSize;
    if(span > 0) {
        s->thumbPosition = (int)((double)s->scrollPosition / maxScroll * span);
        s->contentScrollPosition = (int)((double)s->thumbPosition / span * maxScroll);
    } else {
        s->thumbPosition = 0;
        s->contentScrollPosition = 0;
    }

    if(s->orientation == SCROLLBAR_ORIENTATION_VERTICAL) s->canvas->y = -s->contentScrollPosition;
    else s->canvas->x = -s->contentScrollPosition;
}

static void ScrollBar_BeginDrag(ScrollBar *s, Mouse *mouse, int trackStart) {
    s->dragging = 1;
    s->dragOffset = (s->orientation == SCROLLBAR_ORIENTATION_VERTICAL)
        ? mouse->y - (trackStart + s->thumbPosition)
        : mouse->x - (trackStart + s->thumbPosition);
}

static void ScrollBar_DoDrag(ScrollBar *s, Mouse *mouse, int trackStart, int trackLen) {
    int maxScroll = s->contentSize - s->viewPortSize;
    if(maxScroll <= 0) return;

    int mousePos = (s->orientation == SCROLLBAR_ORIENTATION_VERTICAL) ? mouse->y : mouse->x;
    int span = trackLen - s->thumbSize;
    if(span <= 0) return;

    s->thumbPosition = clamp(mousePos - trackStart - s->dragOffset, 0, span);
    s->scrollPosition = (int)((double)s->thumbPosition / span * maxScroll);
    s->contentScrollPosition = s->scrollPosition;

    if(s->orientation == SCROLLBAR_ORIENTATION_VERTICAL) s->canvas->y = -s->contentScrollPosition;
    else s->canvas->x = -s->contentScrollPosition;
}

static void ScrollBar_Refresh(ScrollBar *s) {
    s->viewPortSize = ScrollBar_GetViewPortSize(s);
    s->trackArea = ScrollBar_GetTrackArea(s);
    s->contentSize = ScrollBar_GetContentSize(s);
    ScrollBar_ApplyThumb(s);
    ScrollBar_UpdateThumbRect(s);
}

ScrollBar *ScrollBar_New(
    Palette *palette, Canvas *canvas,
    ScrollBarOrientation orientation,
    int x, int y, int w, int h,
    int viewPortW, int viewPortH,
    int buttonSize) {

    ScrollBar *s = malloc(sizeof(*s));
    if(!s) return NULL;

    s->orientation = orientation;
    s->x = x;
    s->y = y;
    s->w = w;
    s->h = h;
    s->viewPortW = viewPortW;
    s->viewPortH = viewPortH;
    s->buttonSize = buttonSize;
    s->palette = palette;
    s->canvas = canvas;
    s->scrollPosition = 0;
    s->contentScrollPosition = 0;
    s->thumbPosition = 0;
    s->thumbSize = 0;
    s->viewPortSize = 0;
    s->contentSize = 0;
    s->trackArea = 0;
    s->dragging = 0;
    s->dragOffset = 0;

    if(orientation == SCROLLBAR_ORIENTATION_VERTICAL) {
        s->buttonFirst = Button_New(x, y, w, buttonSize, palette);
        s->buttonSecond = Button_New(x, y + h - buttonSize, w, buttonSize, palette);
    } else {
        s->buttonFirst = Button_New(x, y, buttonSize, h, palette);
        s->buttonSecond = Button_New(x + w - buttonSize, y, buttonSize, h, palette);
    }

    ScrollBar_Refresh(s);
    return s;
}

void ScrollBar_Free(ScrollBar *scrollBar) {
	Button_Free(scrollBar->buttonFirst);
	Button_Free(scrollBar->buttonSecond);
	free(scrollBar);
}

void ScrollBar_Update(ScrollBar *s, Mouse *mouse) {
    ScrollBar_Refresh(s);

    if(Button_Update(s->buttonFirst, mouse)) s->scrollPosition--;
    if(Button_Update(s->buttonSecond, mouse)) s->scrollPosition++;

    int trackStart = ScrollBar_GetTrackStart(s);
    int trackLen = s->trackArea;

    ScrollBar_ApplyThumb(s);

    if(mouse->state & SDL_BUTTON(SDL_BUTTON_LEFT)) {
        if(!s->dragging) {
            if(inrect(mouse->x, mouse->y, s->rectThumb.x, s->rectThumb.y, s->rectThumb.w, s->rectThumb.h)) {
                ScrollBar_BeginDrag(s, mouse, trackStart);
            }
        } else {
            ScrollBar_DoDrag(s, mouse, trackStart, trackLen);
        }
    } else {
        s->dragging = 0;
    }

    ScrollBar_ApplyThumb(s);
    ScrollBar_UpdateThumbRect(s);
}

void ScrollBar_Draw(ScrollBar *s, SDL_Renderer *renderer) {
    SDL_Rect clip = { s->x, s->y, s->w, s->h };
    SDL_RenderSetClipRect(renderer, &clip);

    Button_Draw(s->buttonFirst, renderer);
    Button_Draw(s->buttonSecond, renderer);

    SDL_SetRenderDrawColor(renderer,
        s->palette->colors[13].r,
        s->palette->colors[13].g,
        s->palette->colors[13].b,
        255);
    SDL_RenderFillRect(renderer, &s->rectThumb);

    SDL_SetRenderDrawColor(renderer,
        s->palette->colors[12].r,
        s->palette->colors[12].g,
        s->palette->colors[12].b,
        255);
    SDL_RenderDrawRect(renderer, &clip);

    SDL_RenderSetClipRect(renderer, NULL);
}


/* file: target.c */

#include <stdlib.h>
#include "target.h"

Target *Target_New(Palette *palette, Canvas *canvas, int x, int y) {
    Target *target = malloc(sizeof(*target));

    if(!target) return NULL;

    target->palette = palette;
    target->canvas = canvas;
    target->x = x;
    target->y = y;
    target->cellX = canvas->w/2;
    target->cellY = canvas->h/2;
    target->visible = true;

    return target;
}

void Target_Free(Target *target) {
    free(target);
}

void Target_Update(Target *target, Mouse *mouse) {
    if(mouse->state & SDL_BUTTON_RMASK) {
        int step = target->canvas->gridShow ? target->canvas->pixelSize + 1 : target->canvas->pixelSize;
        if(step <= 0) return;

        int localX = mouse->x - target->canvas->x;
        int localY = mouse->y - target->canvas->y;

        int cellX = localX / step;
        int cellY = localY / step;

        if(localX < 0) cellX--;
        if(localY < 0) cellY--;

        target->cellX = cellX;
        target->cellY = cellY;
    }

    int step = target->canvas->gridShow ? target->canvas->pixelSize + 1 : target->canvas->pixelSize;
    if(step > 0) {
        target->x = target->canvas->x + target->cellX * step + target->canvas->pixelSize / 2;
        target->y = target->canvas->y + target->cellY * step + target->canvas->pixelSize / 2;
    }
}

void Target_Draw(Target *target, SDL_Renderer *renderer) {
    int i, j;
    Uint32 pixel;
    Uint8 r, g, b, a;

    int w = 4 - -4 + 1, h = 4 - -4 + 1;
    SDL_Rect area = { target->x - 4, target->y - 4, w, h };

    SDL_Window *window = SDL_RenderGetWindow(renderer);
    SDL_Surface *surface = SDL_GetWindowSurface(window);
    Uint32 format = SDL_GetWindowPixelFormat(window);
    int pitch = w * SDL_BYTESPERPIXEL(format);
    Uint32 *pixels = malloc(pitch * h);

    if(!pixels) return;

	SDL_Rect clip = {0,0,640-16,480-32-16};
	
	SDL_RenderSetClipRect(renderer,&clip);

    if (SDL_RenderReadPixels(renderer, &area, format, pixels, pitch) == 0) {
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

        for(i = -4; i <= 4; i++) {
            int sx = i + 4;
            int sy = 4;
            pixel = pixels[sy * w + sx];
            SDL_GetRGBA(pixel, surface->format, &r, &g, &b, &a);
            SDL_SetRenderDrawColor(renderer, 255 - r, 255 - g, 255 - b, a);
            SDL_RenderDrawPoint(renderer, target->x + i, target->y);
        }

        for(j = -4; j <= 4; j++) {
            int sx = 4;
            int sy = j + 4;
            pixel = pixels[sy * w + sx];
            SDL_GetRGBA(pixel, surface->format, &r, &g, &b, &a);
            SDL_SetRenderDrawColor(renderer, 255 - r, 255 - g, 255 - b, a);
            SDL_RenderDrawPoint(renderer, target->x, target->y + j);
        }
    }

	SDL_RenderSetClipRect(renderer,NULL);

    free(pixels);
}

/* file: utils.c */

#include "utils.h"

bool inrect(int x, int y, int rx, int ry, int rw, int rh) {
	return x >= rx && x < rx + rw && y >= ry && y < ry + rh;
}

int clamp(int d, int min, int max) {
  const int t = d < min ? min : d;
  return t > max ? max : t;
}

/* file: MyWindow.h */

#ifndef MYWINDOW_H
#define MYWINDOW_H

#include "canvas.h"
#include "scrollbar.h"

typedef struct {
    int x, y, w, h;
    int viewPortW, viewPortH;
    int buttonSize;
    Canvas *canvas;
    ScrollBar *vscroll;
    ScrollBar *hscroll;
} MyWindow;

MyWindow *MyWindow_New(Palette *palette, Canvas *canvas, int x, int y, int w, int h, int boxSize);
void MyWindow_Free(MyWindow *myWindow);

void MyWindow_Update(MyWindow *myWindow, Mouse *mouse);
void MyWindow_Draw(MyWindow *myWindow, SDL_Renderer *renderer);

#endif


/* file: button.h */

#ifndef BUTTON_H
#define BUTTON_H

#include <SDL2/SDL.h>

#include "palette.h"

typedef enum {
	BUTTON_STATE_UP = 0,
	BUTTON_STATE_OVER,
	BUTTON_STATE_DOWN,
	BUTTON_STATE_MAX
} ButtonState;

typedef struct {
	ButtonState state;
	int x,y,w,h;
	Palette *palette;
} Button;

Button *Button_New(int x, int y, int w, int h, Palette *palette);
void Button_Free(Button *button);
void Button_Draw(Button *button, SDL_Renderer *renderer);
bool Button_Update(Button *button, Mouse *mouse);

#endif/* file: canvas.h */

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

typedef struct {
	Palette *palette;
	int x,y,w,h;
	int nframe;
	int transparent;
	byte *pixels;
	byte gridColor;
	bool gridShow;
	int pixelSize;
	int frame;
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

#endif


/* file: common.h */

#ifndef COMMON_H
#define COMMON_H

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

#endif
/* file: mouse.h */

#ifndef MOUSE_H
#define MOUSE_H

#include <SDL2/SDL.h>

typedef struct {
	SDL_Cursor *cursor;
	int x,y;
	Uint32 state;
} Mouse;

Mouse *Mouse_New(char *filename, int x, int y, SDL_Color transparent, int hotSpotX, int hotSpotY);
void Mouse_Free(Mouse *mouse);

void Mouse_EventHandle(Mouse *mouse, SDL_Event event);
void Mouse_Update(Mouse *mouse);

#endif/* file: palette.h */

#ifndef PALETTE_H
#define PALETTE_H

#include "common.h"
#include "types.h"
#include "utils.h"
#include "mouse.h"

typedef struct {
    int x,y,w,h;
    SDL_Color *colors;
    size_t ncolors;
    byte currentColor;
    size_t boxSize;
    bool scrubbing;
} Palette;

Palette *Palette_New(SDL_Color *colors, size_t ncolors, int x, int y, int w, int h, byte currentColor, size_t size);
void Palette_Free(Palette *palette);

void Palette_EventHandle(Palette *palette,SDL_Event event);
void Palette_Update(Palette *palette, Mouse *mouse);
void Palette_Draw(Palette *palette, SDL_Renderer *renderer);

#endif/* file: scrollbar.h */

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
	Palette *palette;
	Canvas *canvas;

	Button *buttonFirst;
	Button *buttonSecond;
	SDL_Rect rectThumb;

	int buttonSize;
	int viewPortW,viewPortH;

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
		int viewPortH, int viewPortW,
		int buttonSize);

void ScrollBar_Free(ScrollBar *scrollBar);

void ScrollBar_Draw(ScrollBar *scrollbar, SDL_Renderer *renderer);
void ScrollBar_Update(ScrollBar *scrollbar, Mouse *mouse);

#endif


/* file: target.h */

#ifndef TARGET_H
#define TARGET_H

#include <SDL2/SDL.h>
#include <stdbool.h>
#include <math.h>

#include "palette.h"
#include "canvas.h"
#include "mouse.h"

typedef struct {
    Palette *palette;
    Canvas *canvas;
    int cellX, cellY;
    int x, y;
    bool visible;
} Target;

Target *Target_New(Palette *palette, Canvas *canvas, int px, int py);
void Target_Free(Target *target);

void Target_Update(Target *target, Mouse *mouse);
void Target_Draw(Target *target, SDL_Renderer *renderer);

#endif


/* file: types.h */

#ifndef TYPE_H
#define TYPE_H

typedef unsigned char byte; 

#endif/* file: utils h */

#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>

bool inrect(int x, int y, int rx, int ry, int rw, int rh);

int clamp(int d, int min, int max);

#endif