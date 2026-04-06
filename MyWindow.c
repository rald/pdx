/* MyWindow.c */

#include <stdlib.h>
#include "MyWindow.h"

MyWindow *MyWindow_New(
		Palette *palette, 
		Canvas *canvas, 
		int x, int y, int w, int h, 
		int buttonSize) {

    MyWindow *myWindow = malloc(sizeof(*myWindow));
    if(!myWindow) return NULL;

    myWindow->x = x;
    myWindow->y = y;
    myWindow->w = w;
    myWindow->h = h;
    myWindow->canvas = canvas;

	myWindow->buttonSize = buttonSize;

    myWindow->viewPortW = w - buttonSize;
    myWindow->viewPortH = h - buttonSize;

    myWindow->vscroll = ScrollBar_New(
        palette, canvas,
        SCROLLBAR_ORIENTATION_VERTICAL,
        x + w - buttonSize, y, buttonSize, h - buttonSize, 
        myWindow->viewPortW, myWindow->viewPortH,
        buttonSize
    );

    myWindow->hscroll = ScrollBar_New(
        palette, canvas,
        SCROLLBAR_ORIENTATION_HORIZONTAL,
        x, y + h - buttonSize, w - buttonSize, buttonSize, 
        myWindow->viewPortW, myWindow->viewPortH,
        buttonSize
    );

    myWindow->canvas->x = x;
    myWindow->canvas->y = y;

    return myWindow;
}

void MyWindow_Update(MyWindow *myWindow, Mouse *mouse) {
    ScrollBar_Update(myWindow->vscroll, mouse);
    ScrollBar_Update(myWindow->hscroll, mouse);
}

void MyWindow_Draw(MyWindow *myWindow, SDL_Renderer *renderer) {
    SDL_Rect viewPort = { myWindow->x, myWindow->y, myWindow->viewPortW, myWindow->viewPortH };
    Canvas_Draw(myWindow->canvas, renderer, viewPort);
    ScrollBar_Draw(myWindow->vscroll, renderer);
    ScrollBar_Draw(myWindow->hscroll, renderer);
}


