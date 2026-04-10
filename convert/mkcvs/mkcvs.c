#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>



int main(int argc,char *argv[]) {

    int w,h,f,t,c,d,i,j,k;
    char n[256];

    printf("n? "); fflush(stdout); scanf("%s",n);
    printf("w? "); fflush(stdout); scanf("%d",&w);
    printf("h? "); fflush(stdout); scanf("%d",&h);
    printf("f? "); fflush(stdout); scanf("%d",&f);
    printf("t? "); fflush(stdout); scanf("%d",&t);
    printf("c? "); fflush(stdout); scanf("%d",&c);
    printf("d? "); fflush(stdout); scanf("%d",&d);
    
    FILE *fp=fopen(n,"w");
    
    if(!fp) {
        fprintf(stderr,"cannot open file %s for writing: %s",n,strerror(errno));
        return EXIT_FAILURE;
    }
    
    fprintf(fp,"%d,%d,%d,%d\n",w,h,f,t);

    for(i=0;i<f;i++) {
        if(i!=0) fprintf(fp,",");
        fprintf(fp,"%d",d);
    }
    fprintf(fp,"\n\n");
    
    for(k=0;k<f;k++) {
        for(j=0;j<h;j++) {
            for(i=0;i<w;i++) {
               fprintf(fp,"%X",c);
            }
            fprintf(fp,"\n");
        }
        fprintf(fp,"\n");
    }
    fprintf(fp,"\n");

    return EXIT_SUCCESS;
}


