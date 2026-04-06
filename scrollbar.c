/* file: scrollbar.c */

#include <stdlib.h>
#include "scrollbar.h"

static int ScrollBar_GetViewPortSize(ScrollBar *s) {
    return s->orientation == SCROLLBAR_ORIENTATION_VERTICAL ? s->viewPortH : s->viewPortW;
}

static int ScrollBar_GetTrackArea(ScrollBar *s) {
    return s->orientation == SCROLLBAR_ORIENTATION_VERTICAL
        ? s->h - s->buttonSize * 2
        : s->w - s->buttonSize * 2;
}

static int ScrollBar_GetContentSize(ScrollBar *s) {
    if(s->orientation == SCROLLBAR_ORIENTATION_VERTICAL) {
        return s->canvas->gridShow
            ? s->canvas->h * (s->canvas->pixelSize + 1) + 1
            : s->canvas->h * s->canvas->pixelSize;
    }
    return s->canvas->gridShow
        ? s->canvas->w * (s->canvas->pixelSize + 1) + 1
        : s->canvas->w * s->canvas->pixelSize;
}

static int ScrollBar_GetTrackStart(ScrollBar *s) {
    return s->orientation == SCROLLBAR_ORIENTATION_VERTICAL ? s->y + s->buttonSize : s->x + s->buttonSize;
}

static void ScrollBar_UpdateThumbRect(ScrollBar *s) {
    if(s->orientation == SCROLLBAR_ORIENTATION_VERTICAL) {
        s->rectThumb = (SDL_Rect){ s->x, s->y + s->buttonSize + s->thumbPosition, s->w, s->thumbSize };
    } else {
        s->rectThumb = (SDL_Rect){ s->x + s->buttonSize + s->thumbPosition, s->y, s->thumbSize, s->h };
    }
}

static void ScrollBar_ApplyThumb(ScrollBar *s) {
    int maxScroll = s->contentSize - s->viewPortSize;

    if(maxScroll <= 0) {
        s->scrollPosition = 0;
        s->thumbPosition = 0;
        s->contentScrollPosition = 0;
        s->thumbSize = s->trackArea;
        if(s->orientation == SCROLLBAR_ORIENTATION_VERTICAL) s->canvas->y = 0;
        else s->canvas->x = 0;
        return;
    }

    s->scrollPosition = clamp(s->scrollPosition, 0, maxScroll);

    s->thumbSize = (int)((double)s->viewPortSize / s->contentSize * s->trackArea);
    s->thumbSize = clamp(s->thumbSize, 1, s->trackArea);

    int span = s->trackArea - s->thumbSize;
    if(span > 0) {
        s->thumbPosition = (int)((double)s->scrollPosition / maxScroll * span);
        s->contentScrollPosition = (int)((double)s->thumbPosition / span * maxScroll);
    } else {
        s->thumbPosition = 0;
        s->contentScrollPosition = 0;
    }

    if(s->orientation == SCROLLBAR_ORIENTATION_VERTICAL) s->canvas->y = -s->contentScrollPosition;
    else s->canvas->x = -s->contentScrollPosition;
}

static void ScrollBar_BeginDrag(ScrollBar *s, Mouse *mouse, int trackStart) {
    s->dragging = 1;
    s->dragOffset = (s->orientation == SCROLLBAR_ORIENTATION_VERTICAL)
        ? mouse->y - (trackStart + s->thumbPosition)
        : mouse->x - (trackStart + s->thumbPosition);
}

static void ScrollBar_DoDrag(ScrollBar *s, Mouse *mouse, int trackStart, int trackLen) {
    int maxScroll = s->contentSize - s->viewPortSize;
    if(maxScroll <= 0) return;

    int mousePos = (s->orientation == SCROLLBAR_ORIENTATION_VERTICAL) ? mouse->y : mouse->x;
    int span = trackLen - s->thumbSize;
    if(span <= 0) return;

    s->thumbPosition = clamp(mousePos - trackStart - s->dragOffset, 0, span);
    s->scrollPosition = (int)((double)s->thumbPosition / span * maxScroll);
    s->contentScrollPosition = s->scrollPosition;

    if(s->orientation == SCROLLBAR_ORIENTATION_VERTICAL) s->canvas->y = -s->contentScrollPosition;
    else s->canvas->x = -s->contentScrollPosition;
}

static void ScrollBar_Refresh(ScrollBar *s) {
    s->viewPortSize = ScrollBar_GetViewPortSize(s);
    s->trackArea = ScrollBar_GetTrackArea(s);
    s->contentSize = ScrollBar_GetContentSize(s);
    ScrollBar_ApplyThumb(s);
    ScrollBar_UpdateThumbRect(s);
}

ScrollBar *ScrollBar_New(
    Palette *palette, Canvas *canvas,
    ScrollBarOrientation orientation,
    int x, int y, int w, int h,
    int viewPortW, int viewPortH,
    int buttonSize) {

    ScrollBar *s = malloc(sizeof(*s));
    if(!s) return NULL;

    s->orientation = orientation;
    s->x = x;
    s->y = y;
    s->w = w;
    s->h = h;
    s->viewPortW = viewPortW;
    s->viewPortH = viewPortH;
    s->buttonSize = buttonSize;
    s->palette = palette;
    s->canvas = canvas;
    s->scrollPosition = 0;
    s->contentScrollPosition = 0;
    s->thumbPosition = 0;
    s->thumbSize = 0;
    s->viewPortSize = 0;
    s->contentSize = 0;
    s->trackArea = 0;
    s->dragging = 0;
    s->dragOffset = 0;

    if(orientation == SCROLLBAR_ORIENTATION_VERTICAL) {
        s->buttonFirst = Button_New(x, y, w, buttonSize, palette);
        s->buttonSecond = Button_New(x, y + h - buttonSize, w, buttonSize, palette);
    } else {
        s->buttonFirst = Button_New(x, y, buttonSize, h, palette);
        s->buttonSecond = Button_New(x + w - buttonSize, y, buttonSize, h, palette);
    }

    ScrollBar_Refresh(s);
    return s;
}

void ScrollBar_Free(ScrollBar *scrollBar) {
	Button_Free(scrollBar->buttonFirst);
	Button_Free(scrollBar->buttonSecond);
	free(scrollBar);
}

void ScrollBar_Update(ScrollBar *s, Mouse *mouse) {
    ScrollBar_Refresh(s);

    if(Button_Update(s->buttonFirst, mouse)) s->scrollPosition--;
    if(Button_Update(s->buttonSecond, mouse)) s->scrollPosition++;

    int trackStart = ScrollBar_GetTrackStart(s);
    int trackLen = s->trackArea;

    ScrollBar_ApplyThumb(s);

    if(mouse->state & SDL_BUTTON(SDL_BUTTON_LEFT)) {
        if(!s->dragging) {
            if(inrect(mouse->x, mouse->y, s->rectThumb.x, s->rectThumb.y, s->rectThumb.w, s->rectThumb.h)) {
                ScrollBar_BeginDrag(s, mouse, trackStart);
            }
        } else {
            ScrollBar_DoDrag(s, mouse, trackStart, trackLen);
        }
    } else {
        s->dragging = 0;
    }

    ScrollBar_ApplyThumb(s);
    ScrollBar_UpdateThumbRect(s);
}

void ScrollBar_Draw(ScrollBar *s, SDL_Renderer *renderer) {
    SDL_Rect clip = { s->x, s->y, s->w, s->h };
    SDL_RenderSetClipRect(renderer, &clip);

    Button_Draw(s->buttonFirst, renderer);
    Button_Draw(s->buttonSecond, renderer);

    SDL_SetRenderDrawColor(renderer,
        s->palette->colors[13].r,
        s->palette->colors[13].g,
        s->palette->colors[13].b,
        255);
    SDL_RenderFillRect(renderer, &s->rectThumb);

    SDL_SetRenderDrawColor(renderer,
        s->palette->colors[12].r,
        s->palette->colors[12].g,
        s->palette->colors[12].b,
        255);
    SDL_RenderDrawRect(renderer, &clip);

    SDL_RenderSetClipRect(renderer, NULL);
}


