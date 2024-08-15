#pragma once

#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdint.h>

#define FPS 30
// FRAME_TARGET_TIME defines minimum interval wait period between two frames in
// millisecond
#define FRAME_TARGET_TIME (1000 / FPS)

enum CULL_METHOD { CULL_NONE, CULL_BACKFACE };
enum RENDER_METHOD {
  RENDER_WIRE,
  RENDER_WIRE_VERTEX,
  RENDER_FILL_TRIANGLE,
  RENDER_FILL_TRIANGLE_WIRE
};

extern int window_width;
extern int window_height;

extern uint32_t *color_buffer;

extern SDL_Window *window;
extern SDL_Renderer *renderer;
extern SDL_Texture *color_buffer_texture;

bool initialize_window(void);
void destroy_window(void);

void draw_grid(void);
void draw_dots(void);
void draw_pixel(int x, int y, uint32_t color);
void draw_line(int x0, int y0, int x1, int y1, uint32_t color);
void draw_rect(int x, int y, int width, int height, uint32_t color);
void draw_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color);

void render_color_buffer(void);
void clear_color_buffer(uint32_t color);