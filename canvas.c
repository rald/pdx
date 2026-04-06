/* file: canvas.c */

#include "canvas.h"

static void Canvas_MouseToCell(Canvas *canvas, int mx, int my, int *cx, int *cy) {
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

void Canvas_EventHandle(Canvas *canvas, SDL_Event event) {
    static bool isDrawing = false;
    static int dx = 0, dy = 0;
    int cx = 0, cy = 0;

    switch(event.type) {
    case SDL_KEYDOWN:
    	switch(event.key.keysym.sym) {
			case SDLK_g:
			canvas->gridShow = !canvas->gridShow;
			break;
			default: break;			
		}
		break;
    case SDL_MOUSEBUTTONDOWN:
        if(!isDrawing && event.button.button == SDL_BUTTON_LEFT) {
            if(canvas->palette && inrect(event.button.x, event.button.y,
                                         canvas->palette->x, canvas->palette->y,
                                         canvas->palette->w, canvas->palette->h)) {
                break;
            }

            Canvas_MouseToCell(canvas, event.button.x, event.button.y, &cx, &cy);
            if(cx >= 0 && cx < canvas->w && cy >= 0 && cy < canvas->h) {
                isDrawing = true;
                dx = cx;
                dy = cy;
            }
        }
        break;

    case SDL_MOUSEBUTTONUP:
        isDrawing = false;
        break;

    case SDL_MOUSEMOTION:
        if(isDrawing) {
            Canvas_MouseToCell(canvas, event.motion.x, event.motion.y, &cx, &cy);
            if(cx >= 0 && cx < canvas->w && cy >= 0 && cy < canvas->h) {
                Canvas_DrawLine(canvas, dx, dy, cx, cy, canvas->palette->currentColor);
                dx = cx;
                dy = cy;
            }
        }
        break;
	case SDL_MOUSEWHEEL:
		if(event.wheel.direction == SDL_MOUSEWHEEL_FLIPPED) {
		    if(event.wheel.y > 0) event.wheel.y *= -1;
		}
		if(canvas->pixelSize > 1 && event.wheel.y < 0) { 
			canvas->pixelSize--; 
		} 
		if(canvas->pixelSize < 32 && event.wheel.y > 0) { 
			canvas->pixelSize++;
		}				
		break;                
    default: 
    	break;
    }
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
