#include "history.h"
#include "canvas.h"
#include <stdlib.h>
#include <string.h>

typedef struct {
    byte* pixels;
    int* delays;
    int nframe;
    int frame;
    int w, h;
} HistoryState;

struct History {
    int capacity;
    HistoryState* undo_stack;
    int undo_top;
    int undo_bottom;
    int undo_count;
    HistoryState* redo_stack;
    int redo_top;
};

static void HistoryState_Free(HistoryState* state) {
    if (state->pixels) {
        free(state->pixels);
        state->pixels = NULL;
    }
    if (state->delays) {
        free(state->delays);
        state->delays = NULL;
    }
}

History* History_New(int capacity) {
    int i;
    History* h = malloc(sizeof(History));
    if (!h) return NULL;
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
        h->undo_stack[i].pixels = NULL;
        h->undo_stack[i].delays = NULL;
        h->redo_stack[i].pixels = NULL;
        h->redo_stack[i].delays = NULL;
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
        HistoryState_Free(&h->undo_stack[i]);
        HistoryState_Free(&h->redo_stack[i]);
    }
    free(h->undo_stack);
    free(h->redo_stack);
    free(h);
}

static void HistoryState_Copy(HistoryState* dest, const Canvas* src) {
    size_t pixel_size = src->w * src->h * src->nframe;
    HistoryState_Free(dest);
    dest->pixels = malloc(pixel_size);
    if (dest->pixels) {
        memcpy(dest->pixels, src->pixels, pixel_size);
    }
    dest->delays = malloc(sizeof(int) * src->nframe);
    if (dest->delays) {
        memcpy(dest->delays, src->delays, sizeof(int) * src->nframe);
    }
    dest->nframe = src->nframe;
    dest->frame = src->frame;
    dest->w = src->w;
    dest->h = src->h;
}

static void Canvas_Restore(Canvas* dest, const HistoryState* src) {
    size_t pixel_size = src->w * src->h * src->nframe;

    free(dest->pixels);
    dest->pixels = malloc(pixel_size);
    if (dest->pixels) {
        memcpy(dest->pixels, src->pixels, pixel_size);
    }

    free(dest->delays);
    dest->delays = malloc(sizeof(int) * src->nframe);
    if (dest->delays) {
        memcpy(dest->delays, src->delays, sizeof(int) * src->nframe);
    }

    dest->nframe = src->nframe;
    dest->frame = src->frame;
    dest->w = src->w;
    dest->h = src->h;
}

void History_Push(History* h, Canvas* canvas) {
    int i;
    if (!h) return;
    
    /* Clear redo stack completely to avoid leaks and match capacity */
    for (i = 0; i < h->capacity; i++) {
        HistoryState_Free(&h->redo_stack[i]);
    }
    h->redo_top = 0;

    /* Push current state to undo stack (circular) */
    HistoryState_Copy(&h->undo_stack[h->undo_top], canvas);

    h->undo_top = (h->undo_top + 1) % h->capacity;
    
    if (h->undo_count < h->capacity) {
        h->undo_count++;
    } else {
        h->undo_bottom = (h->undo_bottom + 1) % h->capacity;
    }
}

int History_Undo(History* h, Canvas* canvas) {
    if (!h || h->undo_count == 0) return 0;

    /* Save current state to redo stack */
    HistoryState_Copy(&h->redo_stack[h->redo_top], canvas);
    h->redo_top++;
    
    /* Pop from undo stack */
    h->undo_top = (h->undo_top - 1 + h->capacity) % h->capacity;
    Canvas_Restore(canvas, &h->undo_stack[h->undo_top]);
    h->undo_count--;
    return 1;
}

int History_Redo(History* h, Canvas* canvas) {
    if (!h || h->redo_top == 0) return 0;

    /* Push current state to undo stack */
    HistoryState_Copy(&h->undo_stack[h->undo_top], canvas);
    h->undo_top = (h->undo_top + 1) % h->capacity;
    if (h->undo_count < h->capacity) {
        h->undo_count++;
    } else {
        h->undo_bottom = (h->undo_bottom + 1) % h->capacity;
    }
    
    /* Pop from redo stack */
    h->redo_top--;
    Canvas_Restore(canvas, &h->redo_stack[h->redo_top]);
    return 1;
}

void History_Clear(History* h) {
    int i;
    if (!h) return;
    for (i = 0; i < h->capacity; i++) {
        HistoryState_Free(&h->undo_stack[i]);
        HistoryState_Free(&h->redo_stack[i]);
    }
    h->undo_top = 0;
    h->undo_bottom = 0;
    h->undo_count = 0;
    h->redo_top = 0;
}
