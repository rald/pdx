/* MyWindow.c */

#include <stdlib.h>
#include "MyWindow.h"

MyWindow *MyWindow_New(Palette *palette, Canvas *canvas, int x, int y, int w, int h) {
    MyWindow *mw = malloc(sizeof(*mw));
    if(!mw) return NULL;

    mw->x = x;
    mw->y = y;
    mw->w = w;
    mw->h = h;
    mw->canvas = canvas;

    mw->viewportW = w - 16;
    mw->viewportH = h - 16;

    mw->canvas->x = x;
    mw->canvas->y = y;

    mw->vscroll = ScrollBar_New(
        palette, canvas,
        SCROLLBAR_ORIENTATION_VERTICAL,
        x + w - 16, y, 16, mw->viewportH
    );

    mw->hscroll = ScrollBar_New(
        palette, canvas,
        SCROLLBAR_ORIENTATION_HORIZONTAL,
        x, y + h - 16, mw->viewportW, 16
    );

    return mw;
}

void MyWindow_Update(MyWindow *mw, Mouse *mouse) {
    ScrollBar_Update(mw->vscroll, mouse);
    ScrollBar_Update(mw->hscroll, mouse);
}

void MyWindow_Draw(MyWindow *mw, SDL_Renderer *renderer) {
    SDL_Rect viewport = { mw->x, mw->y, mw->viewportW, mw->viewportH };
    Canvas_Draw(mw->canvas, renderer, viewport);
    ScrollBar_Draw(mw->vscroll, renderer);
    ScrollBar_Draw(mw->hscroll, renderer);
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
    if(canvas) {
        canvas->palette = palette;
        canvas->w = w;
        canvas->h = h;
        canvas->nframe = nframe;
        canvas->transparent = transparent;
        canvas->pixels = calloc(w * h * nframe, sizeof(*canvas->pixels));
        for(i = 0; i < w * h * nframe; i++) canvas->pixels[i] = color;
        canvas->pixelSize = pixelSize;
        canvas->x = x;
        canvas->y = y;
        canvas->frame = frame;
        canvas->gridColor = gridColor;
        canvas->gridShow = gridShow;
    }
    return canvas;
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

void Canvas_EventHandle(Canvas *canvas,SDL_Event event) {
	switch(event.type) {
	case SDL_KEYDOWN:
		switch(event.key.keysym.sym) {
		case SDLK_g:
			canvas->gridShow = !canvas->gridShow;
			break;	
		default: break;
		}
		break;
	case SDL_MOUSEWHEEL:
		int xScroll = event.wheel.x; 
		int yScroll = event.wheel.y; 
		if (event.wheel.direction == SDL_MOUSEWHEEL_FLIPPED) {
			xScroll *= -1;
			yScroll *= -1;
		}
		
		if(canvas->pixelSize>1 && yScroll<0) {
			canvas->pixelSize-=1;
			yScroll=0;
		}
		
		if(canvas->pixelSize<32 && yScroll>0) {
			canvas->pixelSize+=1;
			yScroll=0;
		}
		
		break;
	default: break;
	}
}

void Canvas_Draw(Canvas *canvas, SDL_Renderer *renderer, SDL_Rect viewport) {

	int i,j,k;
	byte l;

    int px = canvas->pixelSize;
    int step = canvas->gridShow ? px + 1 : px;

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
        for(i = x0; i < x1; i++) {
            k = canvas->frame * canvas->w * canvas->h + j * canvas->w + i;
            l = canvas->pixels[k];
            if(l == canvas->transparent) continue;

            rect.x = canvas->x + i * step + (canvas->gridShow ? 1 : 0);
            rect.y = canvas->y + j * step + (canvas->gridShow ? 1 : 0);
            rect.w = px;
            rect.h = px;

            SDL_SetRenderDrawColor(renderer,
                canvas->palette->colors[l].r,
                canvas->palette->colors[l].g,
                canvas->palette->colors[l].b,
                255);
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
               
               
	palette = Palette_New(colors, ncolors, 0, SCREEN_HEIGHT - 32, 32 * ncolors, 32, 12, 32);

	mouse = Mouse_New("images/mouse.bmp", SCREEN_WIDTH/2, SCREEN_HEIGHT/2, colors[15]);
	SDL_SetCursor(mouse->cursor);
	SDL_WarpMouseInWindow(window, mouse->x, mouse->y);


	canvas=Canvas_LoadCVS("cvs/dog.cvs", palette);
	
	target=Target_New(palette,canvas,0,0);

	myWindow = MyWindow_New(palette, canvas, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT - 32);
	
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
		MyWindow_Update(myWindow, mouse);
		Palette_Update(palette, mouse);	
		Target_Update(target, mouse);
		
		MyWindow_Draw(myWindow, renderer);
		Palette_Draw(palette, renderer);
		Target_Draw(target, renderer);

	    SDL_RenderPresent(renderer);
	    
	    SDL_Delay(10);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();

	return 0;
}


/* file: mouse.c */

#include <stdlib.h>
#include "mouse.h"

Mouse *Mouse_New(char *filename, int x, int y, SDL_Color transparent) {
	Mouse *mouse = malloc(sizeof(*mouse));
	SDL_Surface *surface = NULL;

	if(mouse) {
		mouse->x = x;
		mouse->y = y;
		mouse->state = 0;
		mouse->cursor = NULL;

		surface = SDL_LoadBMP(filename);
		if(surface) {
			SDL_SetColorKey(surface, SDL_TRUE,
				SDL_MapRGB(surface->format, transparent.r, transparent.g, transparent.b));
			mouse->cursor = SDL_CreateColorCursor(surface, 0, 0);
			SDL_FreeSurface(surface);
		}
	}

	return mouse;
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
	if(palette) {
		palette->colors = colors;
		palette->ncolors = ncolors;
		palette->x = x;
		palette->y = y;
		palette->w = w;
		palette->h = h;
		palette->currentColor = currentColor;
		palette->boxSize = boxSize;
	}
	return palette;
}

void Palette_EventHandle(Palette *palette, SDL_Event event) {
}

void Palette_Update(Palette *palette, Mouse *mouse) {
	if(mouse->state & SDL_BUTTON_LMASK) {
		if(inrect(mouse->x,mouse->y,palette->x,palette->y,palette->w,palette->h)) {
			byte i=(mouse->x - palette->x) / palette->boxSize;
			if(i>=0 && i<=palette->ncolors) {
				palette->currentColor=i;
			}
		}
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

static int ScrollBar_GetViewPortSize(ScrollBarOrientation orientation) {
	return orientation == SCROLLBAR_ORIENTATION_VERTICAL
		? 480 - 32 - 16
		: 640 - 16;
}

static int ScrollBar_GetTrackArea(ScrollBarOrientation orientation) {
	return orientation == SCROLLBAR_ORIENTATION_VERTICAL
		? 480 - 32 - 16 - 16 - 16
		: 640 - 16 - 16 - 16;
}

static int ScrollBar_GetContentSize(ScrollBar *scrollBar) {
	if(scrollBar->orientation == SCROLLBAR_ORIENTATION_VERTICAL) {
		return scrollBar->canvas->gridShow
			? scrollBar->canvas->h * (scrollBar->canvas->pixelSize + 1) + 1
			: scrollBar->canvas->h * scrollBar->canvas->pixelSize;
	}

	return scrollBar->canvas->gridShow
		? scrollBar->canvas->w * (scrollBar->canvas->pixelSize + 1) + 1
		: scrollBar->canvas->w * scrollBar->canvas->pixelSize;
}

static int ScrollBar_GetTrackStart(ScrollBar *scrollBar) {
	return scrollBar->orientation == SCROLLBAR_ORIENTATION_VERTICAL
		? scrollBar->y + 16
		: scrollBar->x + 16;
}

static void ScrollBar_UpdateThumbRect(ScrollBar *scrollBar) {
	if(scrollBar->orientation == SCROLLBAR_ORIENTATION_VERTICAL) {
		scrollBar->rectThumb = (SDL_Rect){
			scrollBar->x,
			scrollBar->y + 16 + scrollBar->thumbPosition,
			scrollBar->w,
			scrollBar->thumbSize
		};
	} else {
		scrollBar->rectThumb = (SDL_Rect){
			scrollBar->x + 16 + scrollBar->thumbPosition,
			scrollBar->y,
			scrollBar->thumbSize,
			scrollBar->h
		};
	}
}

static void ScrollBar_ApplyThumb(ScrollBar *scrollBar) {
	int maxScroll = scrollBar->contentSize - scrollBar->viewPortSize;

	if(maxScroll <= 0) {
		scrollBar->scrollPosition = 0;
		scrollBar->thumbPosition = 0;
		scrollBar->contentScrollPosition = 0;
		scrollBar->thumbSize = scrollBar->trackArea;
		if(scrollBar->orientation == SCROLLBAR_ORIENTATION_VERTICAL) scrollBar->canvas->y = 0;
		else scrollBar->canvas->x = 0;
		return;
	}

	scrollBar->scrollPosition = clamp(scrollBar->scrollPosition, 0, maxScroll);

	scrollBar->thumbSize = (int)((double)scrollBar->viewPortSize / scrollBar->contentSize * scrollBar->trackArea);
	scrollBar->thumbSize = clamp(scrollBar->thumbSize, 1, scrollBar->trackArea);

	if(scrollBar->trackArea - scrollBar->thumbSize > 0) {
		scrollBar->thumbPosition =
			(int)((double)scrollBar->scrollPosition / maxScroll * (scrollBar->trackArea - scrollBar->thumbSize));
		scrollBar->contentScrollPosition =
			(int)((double)scrollBar->thumbPosition / (scrollBar->trackArea - scrollBar->thumbSize) * maxScroll);
	} else {
		scrollBar->thumbPosition = 0;
		scrollBar->contentScrollPosition = 0;
	}

	if(scrollBar->orientation == SCROLLBAR_ORIENTATION_VERTICAL) scrollBar->canvas->y = -scrollBar->contentScrollPosition;
	else scrollBar->canvas->x = -scrollBar->contentScrollPosition;
}

static void ScrollBar_BeginDrag(ScrollBar *scrollBar, Mouse *mouse, int trackStart) {
	if(scrollBar->orientation == SCROLLBAR_ORIENTATION_VERTICAL) {
		scrollBar->dragging = 1;
		scrollBar->dragOffset = mouse->y - (trackStart + scrollBar->thumbPosition);
	} else {
		scrollBar->dragging = 1;
		scrollBar->dragOffset = mouse->x - (trackStart + scrollBar->thumbPosition);
	}
}

static void ScrollBar_DoDrag(ScrollBar *scrollBar, Mouse *mouse, int trackStart, int trackLen) {
	int mousePos, newThumbPos, maxScroll;

	maxScroll = scrollBar->contentSize - scrollBar->viewPortSize;
	if(maxScroll <= 0) return;

	mousePos = (scrollBar->orientation == SCROLLBAR_ORIENTATION_VERTICAL) ? mouse->y : mouse->x;

	newThumbPos = clamp(
		mousePos - trackStart - scrollBar->dragOffset,
		0,
		trackLen - scrollBar->thumbSize
	);

	scrollBar->thumbPosition = newThumbPos;
	scrollBar->scrollPosition =
		(int)((double)scrollBar->thumbPosition / (trackLen - scrollBar->thumbSize) * maxScroll);

	scrollBar->contentScrollPosition = scrollBar->scrollPosition;

	if(scrollBar->orientation == SCROLLBAR_ORIENTATION_VERTICAL) {
		scrollBar->canvas->y = -scrollBar->contentScrollPosition;
	} else {
		scrollBar->canvas->x = -scrollBar->contentScrollPosition;
	}
}

static void ScrollBar_Refresh(ScrollBar *scrollBar) {
	scrollBar->viewPortSize = ScrollBar_GetViewPortSize(scrollBar->orientation);
	scrollBar->trackArea = ScrollBar_GetTrackArea(scrollBar->orientation);
	scrollBar->contentSize = ScrollBar_GetContentSize(scrollBar);
	ScrollBar_ApplyThumb(scrollBar);
	ScrollBar_UpdateThumbRect(scrollBar);
}

ScrollBar *ScrollBar_New(
		Palette *palette, Canvas *canvas,
		ScrollBarOrientation orientation,
		int x, int y, int w, int h) {

	ScrollBar *scrollBar = malloc(sizeof(*scrollBar));
	if(!scrollBar) return NULL;

	scrollBar->orientation = orientation;
	scrollBar->x = x;
	scrollBar->y = y;
	scrollBar->w = w;
	scrollBar->h = h;
	scrollBar->palette = palette;
	scrollBar->canvas = canvas;
	scrollBar->scrollPosition = 0;
	scrollBar->contentScrollPosition = 0;
	scrollBar->thumbPosition = 0;
	scrollBar->thumbSize = 0;
	scrollBar->viewPortSize = 0;
	scrollBar->contentSize = 0;
	scrollBar->trackArea = 0;
	scrollBar->dragging = 0;
	scrollBar->dragOffset = 0;

	switch(orientation) {
	case SCROLLBAR_ORIENTATION_VERTICAL:
		scrollBar->buttonFirst = Button_New(x, y, w, 16, palette);
		scrollBar->buttonSecond = Button_New(x, y + h - 16, w, 16, palette);
		break;

	case SCROLLBAR_ORIENTATION_HORIZONTAL:
		scrollBar->buttonFirst = Button_New(x, y, 16, h, palette);
		scrollBar->buttonSecond = Button_New(x + w - 16, y, 16, h, palette);
		break;

	default:
		scrollBar->buttonFirst = NULL;
		scrollBar->buttonSecond = NULL;
		break;
	}

	ScrollBar_Refresh(scrollBar);
	return scrollBar;
}

void ScrollBar_Update(ScrollBar *scrollBar, Mouse *mouse) {
	int trackStart, trackLen, thumbStart, thumbEnd;
	SDL_Rect thumbHit;

	ScrollBar_Refresh(scrollBar);

	switch(scrollBar->orientation) {
	case SCROLLBAR_ORIENTATION_VERTICAL:
		if(Button_Update(scrollBar->buttonFirst, mouse)) scrollBar->scrollPosition--;
		if(Button_Update(scrollBar->buttonSecond, mouse)) scrollBar->scrollPosition++;

		trackStart = ScrollBar_GetTrackStart(scrollBar);
		trackLen = scrollBar->trackArea;

		ScrollBar_ApplyThumb(scrollBar);

		thumbHit = (SDL_Rect){
			scrollBar->x,
			trackStart + scrollBar->thumbPosition,
			scrollBar->w,
			scrollBar->thumbSize
		};

		if(mouse->state & SDL_BUTTON(SDL_BUTTON_LEFT)) {
			if(!scrollBar->dragging) {
				if(inrect(mouse->x, mouse->y, thumbHit.x, thumbHit.y, thumbHit.w, thumbHit.h)) {
					ScrollBar_BeginDrag(scrollBar, mouse, trackStart);
				}
			} else {
				ScrollBar_DoDrag(scrollBar, mouse, trackStart, trackLen);
			}
		} else {
			scrollBar->dragging = 0;
		}

		ScrollBar_ApplyThumb(scrollBar);
		ScrollBar_UpdateThumbRect(scrollBar);
		break;

	case SCROLLBAR_ORIENTATION_HORIZONTAL:
		if(Button_Update(scrollBar->buttonFirst, mouse)) scrollBar->scrollPosition--;
		if(Button_Update(scrollBar->buttonSecond, mouse)) scrollBar->scrollPosition++;

		trackStart = ScrollBar_GetTrackStart(scrollBar);
		trackLen = scrollBar->trackArea;

		ScrollBar_ApplyThumb(scrollBar);

		thumbHit = (SDL_Rect){
			trackStart + scrollBar->thumbPosition,
			scrollBar->y,
			scrollBar->thumbSize,
			scrollBar->h
		};

		if(mouse->state & SDL_BUTTON(SDL_BUTTON_LEFT)) {
			if(!scrollBar->dragging) {
				if(inrect(mouse->x, mouse->y, thumbHit.x, thumbHit.y, thumbHit.w, thumbHit.h)) {
					ScrollBar_BeginDrag(scrollBar, mouse, trackStart);
				}
			} else {
				ScrollBar_DoDrag(scrollBar, mouse, trackStart, trackLen);
			}
		} else {
			scrollBar->dragging = 0;
		}

		ScrollBar_ApplyThumb(scrollBar);
		ScrollBar_UpdateThumbRect(scrollBar);
		break;

	default:
		break;
	}
}

void ScrollBar_Draw(ScrollBar *scrollBar, SDL_Renderer *renderer) {
	SDL_Rect clip = { scrollBar->x, scrollBar->y, scrollBar->w, scrollBar->h };
	SDL_RenderSetClipRect(renderer, &clip);

	SDL_SetRenderDrawColor(renderer,
		scrollBar->palette->colors[12].r,
		scrollBar->palette->colors[12].g,
		scrollBar->palette->colors[12].b,
		255
	);

	SDL_RenderDrawRect(renderer, &clip);

	Button_Draw(scrollBar->buttonFirst, renderer);
	Button_Draw(scrollBar->buttonSecond, renderer);

	SDL_SetRenderDrawColor(renderer,
		scrollBar->palette->colors[13].r,
		scrollBar->palette->colors[13].g,
		scrollBar->palette->colors[13].b,
		255
	);

	SDL_RenderFillRect(renderer, &scrollBar->rectThumb);

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
    target->visible = true;

    return target;
}

void Target_Free(Target *target) {
    free(target);
}


void Target_Update(Target *target, Mouse *mouse) {
    if(mouse->state & SDL_BUTTON_RMASK) {
        int localX = mouse->x - target->canvas->x;
        int localY = mouse->y - target->canvas->y;
        int step = target->canvas->gridShow ? target->canvas->pixelSize + 1 : target->canvas->pixelSize;

        if(step <= 0) return;

        target->x = target->canvas->x + (localX / step) * step + (target->canvas->pixelSize / 2);
        target->y = target->canvas->y + (localY / step) * step + (target->canvas->pixelSize / 2);
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
    int viewportW, viewportH;
    Canvas *canvas;
    ScrollBar *vscroll;
    ScrollBar *hscroll;
} MyWindow;

MyWindow *MyWindow_New(Palette *palette, Canvas *canvas, int x, int y, int w, int h);
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

Canvas *Canvas_LoadCVS(char *filename, Palette *palette);

void Canvas_EventHandle(Canvas *canvas, SDL_Event event);

void Canvas_Draw(Canvas *canvas, SDL_Renderer *renderer, SDL_Rect viewport);

void Canvas_DrawPoint(Canvas *canvas, int x, int y, byte color);
int Canvas_ReadPoint(Canvas *canvas, int x, int y);

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

Mouse *Mouse_New(char *filename, int x, int y, SDL_Color transparent);
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
} Palette;

Palette *Palette_New(SDL_Color *colors, size_t ncolors, int x, int y, int w, int h, byte currentColor, size_t size);

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
		int x, int y, int w, int h);

void ScrollBar_Draw(ScrollBar *scrollbar, SDL_Renderer *renderer);
void ScrollBar_Update(ScrollBar *scrollbar, Mouse *mouse);

#endif/* file: target.h */

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
    int x, y;
    bool visible;
} Target;

Target *Target_New(Palette *palette, Canvas *canvas, int px, int py);
void Target_Free(Target *target);
void Target_Update(Target *target, Mouse *mouse);
void Target_Draw(Target *target, SDL_Renderer *renderer);

#endif/* file: types.h */

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