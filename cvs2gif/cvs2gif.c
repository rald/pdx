#include <SDL2/SDL.h>
#include <string.h>
#include <errno.h>

#include "gifenc.h"

uint8_t colors[] = {
	 26,  28,  44,
	 93,  39,  93,
	177,  62,  83,
	239, 125,  87,
	255, 205, 117,
	167, 240, 112,
	 56, 183, 100,
	 37, 113, 121,
	 41,  54, 111,
	 59,  93, 201,
	 65, 166, 246,
	115, 239, 247,
	244, 244, 244,
	148, 176, 194,
	 86, 108, 134,
	 51,  60,  87
};

uint8_t ncolors = 16;

typedef struct {
    SDL_Color *colors;
    int ncolors;
} Palette;

Palette *Palette_New(uint8_t *colors,uint8_t ncolors) {

    int i;

    Palette *palette=malloc(sizeof(*palette));

    if(!palette) return NULL;

    palette->colors=malloc(sizeof(*palette->colors)*ncolors);

    for(i=0;i<ncolors;i++) {
        palette->colors[i].r = colors[i * 3 + 0];
        palette->colors[i].g = colors[i * 3 + 1];
        palette->colors[i].b = colors[i * 3 + 2];
        palette->colors[i].a = 255;
    }

    return palette;

}

typedef unsigned char byte;

typedef struct {
	int w,h,nframe,transparent;
	byte *pixels;
	int *delays;
} Canvas;

Canvas *Canvas_LoadCVS(char *filename) {
	int i,j,k,l;
	int c;
	char *hex="0123456789ABCDEF";
	Canvas *canvas=malloc(sizeof(*canvas));
	FILE *fp = NULL;

	if(!canvas) return NULL;

	fp=fopen(filename,"rb");
	if(!fp) {
		fprintf(stderr,"Error opening file %s: %s\n",filename,strerror(errno));
		exit(EXIT_FAILURE);
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

	return canvas;
}

int main(int argc, char *argv[]) {
    int f, i, j, k, l, m;

    if(argc!=3) {
        fprintf(stderr,"Syntax: %s filename.cvs filename.gif\n", argv[0]);
        return EXIT_FAILURE;
    }

    Canvas *canvas = Canvas_LoadCVS(argv[1]);

    /* create a GIF */
    ge_GIF *gif = ge_new_gif(
        argv[2],                    /* file name */
        canvas->w, canvas->h,       /* canvas size */
 		colors,			            /* palette */
        4,                          /* palette depth == log2(# of colors) */
        canvas->transparent,        /* transparent color */
        0                           /* infinite loop */
    );

    /* draw some frames */
    for(f = 0; f < canvas->nframe; f++) {
        for (j = 0; j < canvas->h; j++) {
            for (i = 0; i < canvas->w; i++) {
                k = j * canvas->w + i;
                l = f * canvas->w * canvas->h + k;
                gif->frame[k] = canvas->pixels[l];
            }
        }
        ge_add_frame(gif, canvas->delays[f]);
    }

    /* remember to close the GIF */
    ge_close_gif(gif);

    return EXIT_SUCCESS;

}
