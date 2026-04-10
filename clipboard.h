/* file: clipboard.h */

#ifndef CLIPBOARD_H
#define CLIPBOARD_H

#include <stdlib.h>
#include <stdbool.h>

#include "common.h"
#include "canvas.h"

typedef struct Clipboard {
    bool valid;
    int w;
    int h;
    byte *pixels;
} Clipboard;

Clipboard *Clipboard_New(void);
void Clipboard_Free(Clipboard *cb);
void Clipboard_Clear(Clipboard *cb);
bool Clipboard_CopyRect(Clipboard *cb, Canvas *canvas, int x0, int y0, int w, int h);
bool Clipboard_PasteRect(Clipboard *cb, Canvas *canvas, int x0, int y0);

#endif