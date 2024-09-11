#include "swap.h"
#include "display.h"
#include "triangle.h"

void draw_filled_triangle(
  int x0, int y0,
  int x1, int y1,
  int x2, int y2,
  uint32_t color
) {
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

  if (y1 == y2) {
    // We can simply draw the flat-bottom triangle
    fill_flat_bottom_triangle(x0, y0, x1, y1, x2, y2, color);
  } else if (y0 == y1) {
    // We can simply draw the flat-top triangle
    fill_flat_top_triangle(x0, y0, x1, y1, x2, y2, color);
  } else {
    int My = y1;
    int Mx = (((x2 - x0) * (y1 - y0)) / (y2 - y0)) + x0;

    // Draw flat-bottom triangle
    fill_flat_bottom_triangle(x0, y0, x1, y1, Mx, My, color);

    // Draw flat-top triangle
    fill_flat_top_triangle(x1, y1, Mx, My, x2, y2, color);
  }
}

void fill_flat_bottom_triangle(
  int x0, int y0,
  int x1, int y1,
  int x2, int y2,
  uint32_t color
) {
  // Find the two slopes (two triangle legs)
  float inv_slope_1 = (float)(x1 - x0) / (y1 - y0);
  float inv_slope_2 = (float)(x2 - x0) / (y2 - y0);

  // Start x_start and x_end from the top vertex (x0,y0)
  float x_start = x0;
  float x_end = x0;

  // Loop all the scanlines from top to bottom
  for (int y = y0; y <= y2; y++) {
    draw_line(x_start, y, x_end, y, color);
    x_start += inv_slope_1;
    x_end += inv_slope_2;
  }
}

void fill_flat_top_triangle(
  int x0, int y0,
  int x1, int y1,
  int x2, int y2,
  uint32_t color
) {
  // Find the two slopes (two triangle legs)
  float inv_slope_1 = (float)(x2 - x0) / (y2 - y0);
  float inv_slope_2 = (float)(x2 - x1) / (y2 - y1);

  // Start x_start and x_end from the bottom vertex (x2,y2)
  float x_start = x2;
  float x_end = x2;

  // Loop all the scanlines from bottom to top
  for (int y = y2; y >= y0; y--) {
    draw_line(x_start, y, x_end, y, color);
    x_start -= inv_slope_1;
    x_end -= inv_slope_2;
  }
}

vec3_t barycentric_weights(vec2_t a, vec2_t b, vec2_t c, vec2_t p) {
  // Find the vectors between the vertices ABC and point p
  vec2_t ac = vec2_sub(c, a);
  vec2_t ab = vec2_sub(b, a);
  vec2_t ap = vec2_sub(p, a);
  vec2_t pc = vec2_sub(c, p);
  vec2_t pb = vec2_sub(b, p);

  // Compute the area of the full parallegram/triangle ABC using 2D cross product
  float area_parallelogram_abc = (ac.x * ab.y - ac.y * ab.x);

  // Alpha is the area of the small parallelogram/triangle PBC divided by the area of the full parallelogram/triangle ABC
  float alpha = (pc.x * pb.y - pc.y * pb.x) / area_parallelogram_abc;

  // Beta is the area of the small parallelogram/triangle APC divided by the area of the full parallelogram/triangle ABC
  float beta = (ac.x * ap.y - ac.y * ap.x) / area_parallelogram_abc;

  // Weight gamma is easily found since barycentric coordinates always add up to 1.0
  float gamma = 1 - alpha - beta;

  vec3_t weights = { alpha, beta, gamma };

  return weights;
}

void draw_texel(
  int x, int y,
  vec4_t point_a, vec4_t point_b, vec4_t point_c,
  float u0, float v0, float u1, float v1, float u2, float v2,
  uint32_t* texture
) {
  vec2_t a = { point_a.x, point_a.y };
  vec2_t b = { point_b.x, point_b.y };
  vec2_t c = { point_c.x, point_c.y };
  vec2_t p = { x, y };

  vec3_t weights = barycentric_weights(a, b, c, p);

  float alpha = weights.x;
  float beta = weights.y;
  float gamma = weights.z;

  float interpolated_u;
  float interpolated_v;
  float interpolated_reciprocal_w;

  interpolated_u = (u0 / point_a.w) * alpha + (u1 / point_b.w) * beta + (u2 / point_c.w) * gamma;
  interpolated_v = (v0 / point_a.w) * alpha + (v1 / point_b.w) * beta + (v2 / point_c.w) * gamma;

  interpolated_reciprocal_w = (1 / point_a.w) * alpha + (1 / point_b.w) * beta + (1 / point_c.w) * gamma;

  interpolated_u /= interpolated_reciprocal_w;
  interpolated_v /= interpolated_reciprocal_w;
  
  int tex_x = abs((int)(interpolated_u * texture_width));
  int tex_y = abs((int)(interpolated_v * texture_height));

  draw_pixel(x, y, texture[(texture_width * tex_y) + tex_x]);
}

void draw_textured_triangle(
  int x0, int y0, float z0, float w0, float u0, float v0, 
  int x1, int y1, float z1, float w1, float u1, float v1, 
  int x2, int y2, float z2, float w2, float u2, float v2,
  uint32_t* texture
) {
  // We need to sort the vectices by y-coordinate ascending (y0 < y1 < y2)
  if (y0 > y1) {
    int_swap(&y0, &y1);
    int_swap(&x0, &x1);

    float_swap(&u0, &u1);
    float_swap(&v0, &v1);

    float_swap(&z0, &z1);
    float_swap(&w0, &w1);
  }

  if (y1 > y2) {
    int_swap(&y1, &y2);
    int_swap(&x1, &x2);

    float_swap(&u1, &u2);
    float_swap(&v1, &v2);

    float_swap(&z1, &z2);
    float_swap(&w1, &w2);
  }

  if (y0 > y1) {
    int_swap(&y0, &y1);
    int_swap(&x0, &x1);

    float_swap(&u0, &u1);
    float_swap(&v0, &v1);    

    float_swap(&z0, &z1);
    float_swap(&w0, &w1);
  }

  // Create vector points after we sort the vertices
  vec4_t point_a = { x0, y0, z0, w0 };
  vec4_t point_b = { x1, y1, z1, w1 };
  vec4_t point_c = { x2, y2, z2, w2 };

  // Render the upper part of the triangle (flat-bottom)
  float inv_slope_1 = 0;
  float inv_slope_2 = 0;

  if (y1 - y0 != 0) {
    inv_slope_1 = (float)(x1 - x0) / abs(y1 - y0);
  }

  if (y2 - y0 != 0) {
    inv_slope_2 = (float)(x2 - x0) / abs(y2 - y0);
  }

  if (y1 - y0 != 0) {
    for (int y = y0; y <= y1; y++) {
      int x_start = x1 + (y - y1) * inv_slope_1;
      int x_end = x0 + (y - y0) * inv_slope_2;

      if (x_end < x_start) {
        int_swap(&x_start, &x_end);
      }

      for (int x = x_start; x < x_end; x++) {
        draw_texel(
          x, y,
          point_a, point_b, point_c, 
          v0, v0, u1, v1, u2, v2,
          texture
        );
      }
    }
  }

  // Render the bottom part of the triangle (flat-top)
  inv_slope_1 = 0;
  inv_slope_2 = 0;

  if (y2 - y1 != 0) {
    inv_slope_1 = (float)(x2 - x1) / abs(y2 - y1);
  }

  if (y2 - y0 != 0) {
    inv_slope_2 = (float)(x2 - x0) / abs(y2 - y0);
  }

  if (y2 - y1 != 0) {
    for (int y = y1; y <= y2; y++) {
      int x_start = x1 + (y - y1) * inv_slope_1;
      int x_end = x0 + (y - y0) * inv_slope_2;

      if (x_end < x_start) {
        int_swap(&x_start, &x_end);
      }

      for (int x = x_start; x < x_end; x++) {
        draw_texel(
          x, y, 
          point_a, point_b, point_c, 
          v0, v0, u1, v1, u2, v2, 
          texture
        );
      }
    }
  }
}