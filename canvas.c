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
