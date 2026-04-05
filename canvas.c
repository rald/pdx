/* canvas.c */

#include "canvas.h"

Canvas *Canvas_New(
		Palette *palette,
		int x, int y, 
		int w, int h, 
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
		canvas->w = w;
		canvas->h = h;
		canvas->transparent = transparent;

		canvas->pixels = calloc(w * h * nframe,sizeof(*canvas->pixels));
		for(i = 0; i < w * h * nframe; i++) {
			canvas->pixels[i]=color;
		}

		canvas->pixelSize = pixelSize;		
		canvas->x = x;
		canvas->y = y;
		canvas->frame = frame;
		canvas->gridColor = gridColor;
		canvas->gridShow = gridShow;
		
		canvas->palette = palette;
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



