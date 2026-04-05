/* MyWindow.h */
#ifndef MYWINDOW_H
#define MYWINDOW_H

#include "canvas.h"
#include "scrollbar.h"

typedef struct {
    int x, y, w, h;
    Canvas *canvas;
    ScrollBar *vscroll;
    ScrollBar *hscroll;
} MyWindow;

MyWindow *MyWindow_New(Canvas *canvas, int x, int y, int w, int h);
void MyWindow_Draw(MyWindow *myWindow, SDL_Renderer *renderer);

#endif


