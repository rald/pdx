#ifndef HISTORY_H
#define HISTORY_H

#include "types.h"
#include <stddef.h>

struct Canvas;
typedef struct History History;

History* History_New(int capacity);
void History_Free(History* history);
void History_Push(History* history, struct Canvas* canvas);
int History_Undo(History* history, struct Canvas* canvas);
int History_Redo(History* history, struct Canvas* canvas);
void History_Clear(History* history);

#endif
