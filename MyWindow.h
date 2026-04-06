/* file: MyWindow.h */

#ifndef MYWINDOW_H
#define MYWINDOW_H

#include "canvas.h"
#include "scrollbar.h"

typedef struct {
    int x, y, w, h;
    int viewPortW, viewPortH;
    int buttonSize;
    Canvas *canvas;
    ScrollBar *vscroll;
    ScrollBar *hscroll;
} MyWindow;

MyWindow *MyWindow_New(Palette *palette, Canvas *canvas, int x, int y, int w, int h, int boxSize);
void MyWindow_Free(MyWindow *myWindow);

void MyWindow_Update(MyWindow *myWindow, Mouse *mouse);
void MyWindow_Draw(MyWindow *myWindow, SDL_Renderer *renderer);

#endif


