/* MyWindow.c */

#include <stdlib.h>
#include "MyWindow.h"

MyWindow *MyWindow_New(Palette *palette, Canvas *canvas, int x, int y, int w, int h) {
    MyWindow *mw = malloc(sizeof(*mw));
    if(!mw) return NULL;

    mw->x = x;
    mw->y = y;
    mw->w = w;
    mw->h = h;
    mw->canvas = canvas;

    mw->viewportW = w - 16;
    mw->viewportH = h - 16;

    mw->canvas->x = x;
    mw->canvas->y = y;

    mw->vscroll = ScrollBar_New(
        palette, canvas,
        SCROLLBAR_ORIENTATION_VERTICAL,
        x + w - 16, y, 16, mw->viewportH, 
        16
    );

    mw->hscroll = ScrollBar_New(
        palette, canvas,
        SCROLLBAR_ORIENTATION_HORIZONTAL,
        x, y + h - 16, mw->viewportW, 16, 
        16
    );

    return mw;
}

void MyWindow_Update(MyWindow *mw, Mouse *mouse) {
    ScrollBar_Update(mw->vscroll, mouse);
    ScrollBar_Update(mw->hscroll, mouse);
}

void MyWindow_Draw(MyWindow *mw, SDL_Renderer *renderer) {
    SDL_Rect viewport = { mw->x, mw->y, mw->viewportW, mw->viewportH };
    Canvas_Draw(mw->canvas, renderer, viewport);
    ScrollBar_Draw(mw->vscroll, renderer);
    ScrollBar_Draw(mw->hscroll, renderer);
}


