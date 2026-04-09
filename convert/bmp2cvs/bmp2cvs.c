#include <SDL2/SDL.h>

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

int main(int argc,char *argv[]) {

	int i,j;

	if(argc!=3) {
		fprintf(stderr,"Syntax: %s [filename.bmp] [filename.cvs]\n",argv[0]);
		return EXIT_FAILURE;
	}

	SDL_Surface *surface=SDL_LoadBMP(argv[1]);

	if(surface==NULL) {
		printf("Unable to load bitmap: %s\n",SDL_GetError());
		return EXIT_FAILURE;
	}

	SDL_LockSurface(surface);

	FILE *fp=fopen(argv[2],"w");

	fprintf(fp,"%d,%d,1,-1\n",surface->w,surface->h);
	fprintf(fp,"100\n\n");
	for(j=0;j<surface->h;j++) {
		for(i=0;i<surface->w;i++) {
			Uint32 pixel=getpixel(surface,i,j);
			if(pixel>=0 && pixel<16) {
				fprintf(fp,"%X",pixel);
			} else {
				printf("File %s: %d: %d Error: invalid pixel color\n",argv[2],i,j);
				return EXIT_FAILURE;
			}
		}
		fprintf(fp,"\n");
	}
	fprintf(fp,"\n");
	fclose(fp);

	SDL_UnlockSurface(surface);

	return EXIT_SUCCESS;
}
