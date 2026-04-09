#ifndef HISTORY_H
#define HISTORY_H

#include "types.h"
#include <stddef.h>

typedef struct History History;

History* History_New(size_t buffer_size, int capacity);
void History_Free(History* history);
void History_Push(History* history, const byte* buffer);
int History_Undo(History* history, byte* current_buffer);
int History_Redo(History* history, byte* current_buffer);
void History_Clear(History* history);

#endif
