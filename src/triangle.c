#include "triangle.h"

void int_swap(int* a, int* b) {
  int temp = *a;
  *a = *b;
  *b = temp;
}

void draw_filled_triangle(int x0, int y0, int x1, int y1, int x2, int y2,
                          int32_t color) {
  // We need to sort the vectices by y-coordinate ascending (y0 < y1 < y2)
  if (y0 > y1) {
    int_swap(&y0, &y1);
    int_swap(&x0, &x1);
  }

  if (y1 > y2) {
    int_swap(&y1, &y2);
    int_swap(&x1, &x2);
  }

  if (y0 > y1) {
    int_swap(&y0, &y1);
    int_swap(&x0, &x1);
  }

  int My = y1;
  int Mx = ((float)((x2 - x0) * (y1 - y0)) / (float)(y2 - y0)) + x0;

  // Draw flat-bottom triangle
  fill_flat_bottom_triangle(x0, y0, x1, y1, Mx, My);

  // Draw flat-top triangle
  fill_top_bottom_triangle(x1, y1, Mx, My, x2, y2);
}