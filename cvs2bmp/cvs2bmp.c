#include <SDL2/SDL.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>


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



typedef unsigned char byte;



typedef struct {
	int w,h,nframe,transparent;
	byte *pixels;
} Canvas;

void putpixel(SDL_Surface *surface, int x, int y, Uint32 pixel) {
    int bpp = surface->format->BytesPerPixel;
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;
    /* Handle 1, 2, 3, or 4 bytes per pixel... */
    switch(bpp) {
        case 1: *p = pixel; break;
        case 2: *(Uint16 *)p = pixel; break;
        case 3: /* Handle RGB/BGR based on endianness */ break;
        case 4: *(Uint32 *)p = pixel; break;
    }
}

Uint32 getpixel(SDL_Surface *surface, int x, int y) {
    int bpp = surface->format->BytesPerPixel;

    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

    switch (bpp) {
        case 1:
            return *p;
        case 2:
            return *(Uint16 *)p;
        case 3:
            if (SDL_BYTEORDER == SDL_BIG_ENDIAN) {
                return p[0] << 16 | p[1] << 8 | p[2];
            } else {
                return p[0] | p[1] << 8 | p[2] << 16;
            }
        case 4:
            return *(Uint32 *)p;
        default:
            return 0; /* Should not happen */
    }
}

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

	return canvas;
}

int main(int argc,char *argv[]) {

	int i,j,k;
	int f;
	char filename[256];

	if(argc!=3) {
		fprintf(stderr,"Syntax: %s [filename.cvs] [filename]\n",argv[0]);
		return EXIT_FAILURE;
	}

	Canvas *canvas=Canvas_LoadCVS(argv[1]);


	for(f=0;f<canvas->nframe;f++) {	

		SDL_Surface* surface = SDL_CreateRGBSurface(0, canvas->w, canvas->h, 32, 0, 0, 0, 0);

		if (surface == NULL) {
			fprintf(stderr,"Error: Unable to create surface: %s\n", SDL_GetError());
			return EXIT_FAILURE;		
		}

		for(j=0;j<canvas->h;j++) {
			for(i=0;i<canvas->w;i++) {
				k=canvas->pixels[f*canvas->w*canvas->h+j*canvas->w+i];
				putpixel(surface,i,j,SDL_MapRGBA(surface->format,colors[k].r,colors[k].g,colors[k].b,255));
			}
		}

		sprintf(filename,"%s.%d.bmp",argv[2],f);

		if(SDL_SaveBMP(surface,filename)!=0) {
			fprintf(stderr,"Error: Failed to save BMP: %s\n", SDL_GetError());
			return EXIT_FAILURE;
		}


		SDL_FreeSurface(surface);
	}


	return EXIT_SUCCESS;
}
