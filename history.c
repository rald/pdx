#include "history.h"
#include <stdlib.h>
#include <string.h>

typedef struct {
    byte* data;
} HistoryState;

struct History {
    size_t buffer_size;
    int capacity;
    HistoryState* undo_stack;
    int undo_top;
    int undo_bottom;
    int undo_count;
    HistoryState* redo_stack;
    int redo_top;
};

History* History_New(size_t buffer_size, int capacity) {
    int i, j;
    History* h = malloc(sizeof(History));
    if (!h) return NULL;
    h->buffer_size = buffer_size;
    h->capacity = capacity;
    h->undo_stack = malloc(sizeof(HistoryState) * capacity);
    if (!h->undo_stack) {
        free(h);
        return NULL;
    }
    h->redo_stack = malloc(sizeof(HistoryState) * capacity);
    if (!h->redo_stack) {
        free(h->undo_stack);
        free(h);
        return NULL;
    }
    for (i = 0; i < capacity; i++) {
        h->undo_stack[i].data = malloc(buffer_size);
        if (!h->undo_stack[i].data) {
            for (j = 0; j < i; j++) free(h->undo_stack[j].data);
            free(h->undo_stack);
            free(h->redo_stack);
            free(h);
            return NULL;
        }
        h->redo_stack[i].data = malloc(buffer_size);
        if (!h->redo_stack[i].data) {
            for (j = 0; j <= i; j++) free(h->undo_stack[j].data);
            for (j = 0; j < i; j++) free(h->redo_stack[j].data);
            free(h->undo_stack);
            free(h->redo_stack);
            free(h);
            return NULL;
        }
    }
    h->undo_top = 0;
    h->undo_bottom = 0;
    h->undo_count = 0;
    h->redo_top = 0;
    return h;
}

void History_Free(History* h) {
    int i;
    if (!h) return;
    for (i = 0; i < h->capacity; i++) {
        free(h->undo_stack[i].data);
        free(h->redo_stack[i].data);
    }
    free(h->undo_stack);
    free(h->redo_stack);
    free(h);
}

void History_Push(History* h, const byte* buffer) {
    if (!h) return;

    /* Push current state to undo stack (circular) */
    memcpy(h->undo_stack[h->undo_top].data, buffer, h->buffer_size);
    h->undo_top = (h->undo_top + 1) % h->capacity;

    if (h->undo_count < h->capacity) {
        h->undo_count++;
    } else {
        h->undo_bottom = (h->undo_bottom + 1) % h->capacity;
    }

    /* Clear redo stack */
    h->redo_top = 0;
}

int History_Undo(History* h, byte* current_buffer) {
    if (!h || h->undo_count == 0) return 0;

    /* Save current state to redo stack (simple stack, capacity matches) */
    memcpy(h->redo_stack[h->redo_top].data, current_buffer, h->buffer_size);
    h->redo_top++;

    /* Pop from undo stack */
    h->undo_top = (h->undo_top - 1 + h->capacity) % h->capacity;
    memcpy(current_buffer, h->undo_stack[h->undo_top].data, h->buffer_size);
    h->undo_count--;
    return 1;
}

int History_Redo(History* h, byte* current_buffer) {
    if (!h || h->redo_top == 0) return 0;

    /* Push current state to undo stack */
    memcpy(h->undo_stack[h->undo_top].data, current_buffer, h->buffer_size);
    h->undo_top = (h->undo_top + 1) % h->capacity;
    if (h->undo_count < h->capacity) {
        h->undo_count++;
    } else {
        h->undo_bottom = (h->undo_bottom + 1) % h->capacity;
    }

    /* Pop from redo stack */
    h->redo_top--;
    memcpy(current_buffer, h->redo_stack[h->redo_top].data, h->buffer_size);
    return 1;
}

void History_Clear(History* h) {
    if (!h) return;
    h->undo_top = 0;
    h->undo_bottom = 0;
    h->undo_count = 0;
    h->redo_top = 0;
}
