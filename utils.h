/* file: utils h */

#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>

bool inrect(int x, int y, int rx, int ry, int rw, int rh);

int clamp(int d, int min, int max);

#endif