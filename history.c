#include "history.h"
#include "canvas.h"
#include <stdlib.h>
#include <string.h>

typedef struct {
    byte* pixels;
    int* delays;
    int nframe;
    int frame;
    int transparent;
} Snapshot;

struct History {
    int w, h;
    int capacity;
    Snapshot* undo_stack;
    int undo_top;
    int undo_bottom;
    int undo_count;
    Snapshot* redo_stack;
    int redo_top;
};

static void Snapshot_Clear(Snapshot* s) {
    if (s->pixels) free(s->pixels);
    if (s->delays) free(s->delays);
    s->pixels = NULL;
    s->delays = NULL;
    s->nframe = 0;
}

static int Snapshot_Copy(Snapshot* s, struct Canvas* canvas) {
    size_t pixel_size = canvas->w * canvas->h * canvas->nframe;
    s->pixels = malloc(pixel_size);
    if (!s->pixels) return 0;
    memcpy(s->pixels, canvas->pixels, pixel_size);

    s->delays = malloc(sizeof(int) * canvas->nframe);
    if (!s->delays) {
        free(s->pixels);
        s->pixels = NULL;
        return 0;
    }
    memcpy(s->delays, canvas->delays, sizeof(int) * canvas->nframe);

    s->nframe = canvas->nframe;
    s->frame = canvas->frame;
    s->transparent = canvas->transparent;
    return 1;
}

static int Snapshot_Restore(Snapshot* s, struct Canvas* canvas) {
    byte* new_pixels = realloc(canvas->pixels, canvas->w * canvas->h * s->nframe);
    if (!new_pixels) return 0;
    canvas->pixels = new_pixels;

    int* new_delays = realloc(canvas->delays, sizeof(int) * s->nframe);
    if (!new_delays) return 0;
    canvas->delays = new_delays;

    memcpy(canvas->pixels, s->pixels, canvas->w * canvas->h * s->nframe);
    memcpy(canvas->delays, s->delays, sizeof(int) * s->nframe);
    canvas->nframe = s->nframe;
    canvas->frame = s->frame;
    canvas->transparent = s->transparent;
    return 1;
}

History* History_New(int w, int h, int capacity) {
    History* hist = malloc(sizeof(History));
    if (!hist) return NULL;
    hist->w = w;
    hist->h = h;
    hist->capacity = capacity;
    hist->undo_stack = calloc(capacity, sizeof(Snapshot));
    hist->redo_stack = calloc(capacity, sizeof(Snapshot));
    if (!hist->undo_stack || !hist->redo_stack) {
        free(hist->undo_stack);
        free(hist->redo_stack);
        free(hist);
        return NULL;
    }
    hist->undo_top = 0;
    hist->undo_bottom = 0;
    hist->undo_count = 0;
    hist->redo_top = 0;
    return hist;
}

void History_Free(History* h) {
    int i;
    if (!h) return;
    for (i = 0; i < h->capacity; i++) {
        Snapshot_Clear(&h->undo_stack[i]);
        Snapshot_Clear(&h->redo_stack[i]);
    }
    free(h->undo_stack);
    free(h->redo_stack);
    free(h);
}

void History_Push(History* h, struct Canvas* canvas) {
    if (!h) return;
    
    /* Clear current redo stack because we're doing a new action */
    while (h->redo_top > 0) {
        h->redo_top--;
        Snapshot_Clear(&h->redo_stack[h->redo_top]);
    }

    /* If undo stack is full, clear the oldest entry */
    if (h->undo_count == h->capacity) {
        Snapshot_Clear(&h->undo_stack[h->undo_bottom]);
        h->undo_bottom = (h->undo_bottom + 1) % h->capacity;
    } else {
        h->undo_count++;
    }

    /* Push current state to undo stack */
    Snapshot_Copy(&h->undo_stack[h->undo_top], canvas);
    h->undo_top = (h->undo_top + 1) % h->capacity;
}

int History_Undo(History* h, struct Canvas* canvas) {
    if (!h || h->undo_count == 0) return 0;

    /* Push current state to redo stack */
    Snapshot_Copy(&h->redo_stack[h->redo_top], canvas);
    h->redo_top++;
    
    /* Pop from undo stack and restore */
    h->undo_top = (h->undo_top - 1 + h->capacity) % h->capacity;
    Snapshot_Restore(&h->undo_stack[h->undo_top], canvas);
    Snapshot_Clear(&h->undo_stack[h->undo_top]);
    h->undo_count--;
    return 1;
}

int History_Redo(History* h, struct Canvas* canvas) {
    if (!h || h->redo_top == 0) return 0;

    /* Push current state to undo stack */
    if (h->undo_count == h->capacity) {
        Snapshot_Clear(&h->undo_stack[h->undo_bottom]);
        h->undo_bottom = (h->undo_bottom + 1) % h->capacity;
    } else {
        h->undo_count++;
    }
    Snapshot_Copy(&h->undo_stack[h->undo_top], canvas);
    h->undo_top = (h->undo_top + 1) % h->capacity;
    
    /* Pop from redo stack and restore */
    h->redo_top--;
    Snapshot_Restore(&h->redo_stack[h->redo_top], canvas);
    Snapshot_Clear(&h->redo_stack[h->redo_top]);
    return 1;
}

void History_Clear(History* h) {
    int i;
    if (!h) return;
    for (i = 0; i < h->capacity; i++) {
        Snapshot_Clear(&h->undo_stack[i]);
        Snapshot_Clear(&h->redo_stack[i]);
    }
    h->undo_top = 0;
    h->undo_bottom = 0;
    h->undo_count = 0;
    h->redo_top = 0;
}
