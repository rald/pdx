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

void Canvas_Update(Canvas *canvas, Mouse *mouse) {
}

void Canvas_Draw(Canvas *canvas, SDL_Renderer *renderer) {
	int i,j,k,l;

	SDL_Rect rect;

	SDL_Rect clip = { 0, 0, SCREEN_WIDTH - 16, SCREEN_HEIGHT - 32 - 16 };
	SDL_RenderSetClipRect(renderer, &clip);

	if(canvas->gridShow) {
		rect.x = canvas->x;
		rect.y = canvas->y;
		rect.w = canvas->w * (canvas->pixelSize + 1) + 1;
		rect.h = canvas->h * (canvas->pixelSize + 1) + 1;

		SDL_SetRenderDrawColor(
			renderer, 
			canvas->palette->colors[canvas->gridColor].r, 
			canvas->palette->colors[canvas->gridColor].g, 
			canvas->palette->colors[canvas->gridColor].b, 
			255);

		SDL_RenderFillRect(renderer, &rect);
	}

	for(j = 0; j < canvas->h; j++) {
		for(i = 0; i < canvas->w; i++) {

			k = canvas->frame * canvas->w * canvas->h + j * canvas->w + i;
			l = canvas->pixels[k];

			if(l != canvas->transparent) {
								
				if(canvas->gridShow) {
					rect.x = i * (canvas->pixelSize + 1) + canvas->x + 1;
					rect.y = j * (canvas->pixelSize + 1) + canvas->y + 1;
					rect.w = canvas->pixelSize;
					rect.h = canvas->pixelSize;
				} else {
					rect.x = i * canvas->pixelSize + canvas->x;
					rect.y = j * canvas->pixelSize + canvas->y;
					rect.w = canvas->pixelSize;
					rect.h = canvas->pixelSize;
				}
				
				if(rect.x>=SCREEN_WIDTH) break;

				SDL_SetRenderDrawColor(renderer, canvas->palette->colors[l].r, canvas->palette->colors[l].g, canvas->palette->colors[l].b, 255);
				SDL_RenderFillRect(renderer, &rect);
			}
		}
		
		if(rect.y>=SCREEN_HEIGHT) break;
	}

	SDL_RenderSetClipRect(renderer, NULL);
}

void Canvas_DrawPoint(Canvas *canvas, int x, int y, byte color) {
	if(x>=0 && x<canvas->w && y>=0 && y<canvas->h) {
		canvas->pixels[canvas->frame*canvas->w*canvas->h+canvas->y*canvas->w+canvas->x]=color;
	}
}

int Canvas_ReadPoint(Canvas *canvas, int x, int y) {
	if(x>=0 && x<canvas->w && y>=0 && y<canvas->h) {
		return canvas->pixels[canvas->frame*canvas->w*canvas->h+canvas->y*canvas->w+canvas->x];
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



