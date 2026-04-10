/* file: clipboard.c */

#include "clipboard.h"

Clipboard *Clipboard_New(void) {
    Clipboard *cb = (Clipboard *)malloc(sizeof(Clipboard));
    if(!cb) return NULL;
    cb->valid = false;
    cb->w = 0;
    cb->h = 0;
    cb->pixels = NULL;
    return cb;
}

void Clipboard_Clear(Clipboard *cb) {
    if(cb->pixels) free(cb->pixels);
    cb->pixels = NULL;
    cb->valid = false;
    cb->w = 0;
    cb->h = 0;
}

void Clipboard_Free(Clipboard *cb) {
    if(!cb) return;
    Clipboard_Clear(cb);
    free(cb);
}

bool Clipboard_CopyRect(Clipboard *cb, Canvas *canvas, int x0, int y0, int w, int h) {
    int x, y;

    if(!cb || !canvas || w <= 0 || h <= 0) return false;

    Clipboard_Clear(cb);

    cb->pixels = (byte *)malloc(w * h);
    if(!cb->pixels) return false;

    cb->w = w;
    cb->h = h;

    for(y = 0; y < h; y++) {
        for(x = 0; x < w; x++) {
            cb->pixels[y * w + x] = Canvas_ReadPoint(canvas, x0 + x, y0 + y);
        }
    }

    cb->valid = true;
    return true;
}

bool Clipboard_PasteRect(Clipboard *cb, Canvas *canvas, int x0, int y0) {
    int x, y;

    if(!cb || !canvas || !cb->valid || !cb->pixels) return false;

    for(y = 0; y < cb->h; y++) {
        for(x = 0; x < cb->w; x++) {
            Canvas_DrawPoint(canvas, x0 + x, y0 + y, cb->pixels[y * cb->w + x]);
        }
    }

    return true;
}