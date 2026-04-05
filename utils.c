/* file: utils.c */

#include "utils.h"

bool inrect(int x, int y, int rx, int ry, int rw, int rh) {
	return x >= rx && x < rx + rw && y >= ry && y < ry + rh;
}

int clamp(int d, int min, int max) {
  const int t = d < min ? min : d;
  return t > max ? max : t;
}

