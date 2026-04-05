/* MyWindow.c */
#include <stdlib.h>
#include "MyWindow.h"

MyWindow *MyWindow_New(Canvas *canvas, int x, int y, int w, int h) {
    MyWindow *myWindow = malloc(sizeof(*myWindow));
    if(!myWindow) return NULL;

    myWindow->x = x;
    myWindow->y = y;
    myWindow->w = w;
    myWindow->h = h;
    myWindow->canvas = canvas;

    myWindow->vscroll = ScrollBar_New(canvas->palette, canvas, SCROLLBAR_ORIENTATION_VERTICAL, x + w - 16, y, 16, h);
    myWindow->hscroll = ScrollBar_New(canvas->palette, canvas, SCROLLBAR_ORIENTATION_HORIZONTAL, x, y + h - 16, w, 16);

    return myWindow;
}

void MyWindow_Draw(MyWindow *myWindow, SDL_Renderer *renderer) {
    SDL_Rect viewport = { myWindow->x, myWindow->y, myWindow->w, myWindow->h };
    Canvas_Draw(myWindow->canvas, renderer, viewport);
    ScrollBar_Draw(myWindow->vscroll, renderer);
    ScrollBar_Draw(myWindow->hscroll, renderer);
}


