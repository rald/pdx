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

static void FreeState(HistoryState* state) {
    if (state->pixels) free(state->pixels);
    if (state->delays) free(state->delays);
    state->pixels = NULL;
    state->delays = NULL;
}

static int CopyCanvasToState(HistoryState* state, struct Canvas* canvas) {
    size_t pixel_size = canvas->w * canvas->h * canvas->nframe;
    state->pixels = malloc(pixel_size);
    if (!state->pixels) return 0;
    memcpy(state->pixels, canvas->pixels, pixel_size);

    state->delays = malloc(sizeof(int) * canvas->nframe);
    if (!state->delays) {
        free(state->pixels);
        state->pixels = NULL;
        return 0;
    }
    memcpy(state->delays, canvas->delays, sizeof(int) * canvas->nframe);

    state->nframe = canvas->nframe;
    state->frame = canvas->frame;
    state->w = canvas->w;
    state->h = canvas->h;
    return 1;
}

static void RestoreCanvasFromState(struct Canvas* canvas, HistoryState* state) {
    size_t pixel_size = state->w * state->h * state->nframe;

    byte* new_pixels = realloc(canvas->pixels, pixel_size);
    if (new_pixels) {
        canvas->pixels = new_pixels;
        memcpy(canvas->pixels, state->pixels, pixel_size);
    }

    int* new_delays = realloc(canvas->delays, sizeof(int) * state->nframe);
    if (new_delays) {
        canvas->delays = new_delays;
        memcpy(canvas->delays, state->delays, sizeof(int) * state->nframe);
    }

    if (new_pixels && new_delays) {
        canvas->nframe = state->nframe;
        canvas->frame = state->frame;
        canvas->w = state->w;
        canvas->h = state->h;
    }
}

History* History_New(int capacity) {
    History* h = calloc(1, sizeof(History));
    if (!h) return NULL;
    h->capacity = capacity;
    h->undo_stack = calloc(capacity, sizeof(HistoryState));
    if (!h->undo_stack) {
        free(h);
        return NULL;
    }
    h->redo_stack = calloc(capacity, sizeof(HistoryState));
    if (!h->redo_stack) {
        free(h->undo_stack);
        free(h);
        return NULL;
    }
    return h;
}

void History_Free(History* h) {
    int i;
    if (!h) return;
    for (i = 0; i < h->capacity; i++) {
        FreeState(&h->undo_stack[i]);
        FreeState(&h->redo_stack[i]);
    }
    free(h->undo_stack);
    free(h->redo_stack);
    free(h);
}

void History_Push(History* h, struct Canvas* canvas) {
    if (!h) return;
    
    /* Clear redo stack */
    while (h->redo_top > 0) {
        h->redo_top--;
        FreeState(&h->redo_stack[h->redo_top]);
    }

    /* If we're at capacity, free the oldest undo state */
    if (h->undo_count == h->capacity) {
        FreeState(&h->undo_stack[h->undo_bottom]);
        h->undo_bottom = (h->undo_bottom + 1) % h->capacity;
    } else {
        h->undo_count++;
    }

    /* Push current state to undo stack */
    if (CopyCanvasToState(&h->undo_stack[h->undo_top], canvas)) {
        h->undo_top = (h->undo_top + 1) % h->capacity;
    }
}

int History_Undo(History* h, struct Canvas* canvas) {
    if (!h || h->undo_count == 0) return 0;

    /* Save current state to redo stack */
    if (!CopyCanvasToState(&h->redo_stack[h->redo_top], canvas)) {
        return 0;
    }
    h->redo_top++;
    
    /* Pop from undo stack */
    h->undo_top = (h->undo_top - 1 + h->capacity) % h->capacity;
    RestoreCanvasFromState(canvas, &h->undo_stack[h->undo_top]);
    FreeState(&h->undo_stack[h->undo_top]);
    h->undo_count--;
    return 1;
}

int History_Redo(History* h, struct Canvas* canvas) {
    if (!h || h->redo_top == 0) return 0;

    /* Push current state back to undo stack */
    if (!CopyCanvasToState(&h->undo_stack[h->undo_top], canvas)) {
        return 0;
    }
    h->undo_top = (h->undo_top + 1) % h->capacity;
    if (h->undo_count < h->capacity) {
        h->undo_count++;
    } else {
        /* This should ideally not happen if push/undo are balanced, but for safety: */
        FreeState(&h->undo_stack[h->undo_bottom]);
        h->undo_bottom = (h->undo_bottom + 1) % h->capacity;
    }
    
    /* Pop from redo stack */
    h->redo_top--;
    RestoreCanvasFromState(canvas, &h->redo_stack[h->redo_top]);
    FreeState(&h->redo_stack[h->redo_top]);
    return 1;
}

void History_Clear(History* h) {
    int i;
    if (!h) return;
    for (i = 0; i < h->capacity; i++) {
        FreeState(&h->undo_stack[i]);
        FreeState(&h->redo_stack[i]);
    }
    h->undo_top = 0;
    h->undo_bottom = 0;
    h->undo_count = 0;
    h->redo_top = 0;
}
