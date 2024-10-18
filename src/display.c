#include "display.h"

#include <stdio.h>
#include <stdbool.h>
#include <SDL2/SDL.h>

static int window_width = 800;
static int window_height = 600;

static float* z_buffer = NULL;
static uint32_t *color_buffer = NULL;

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static SDL_Texture *color_buffer_texture = NULL;

static enum CULL_METHOD cull_method = CULL_BACKFACE;
static enum RENDER_METHOD render_method = RENDER_WIRE_VERTEX;

int get_window_width(void) {
  return window_width;
}

int get_window_height(void) {
  return window_height;
}

void set_render_method(int method) {
  render_method = method;
}

void set_cull_method(int method) {
  cull_method = method;
}

bool is_back_culling(void) {
  return cull_method == CULL_BACKFACE;
}

bool should_render_filled_triangle() {
  return (
    render_method == RENDER_FILL_TRIANGLE ||
    render_method == RENDER_FILL_TRIANGLE_WIRE
  );
}

bool should_render_textured_triangle(void) {
  return (
    render_method == RENDER_TEXTURED ||
    render_method == RENDERED_TEXTURED_WIRE
  );
}

bool should_render_wireframe(void) {
  return (
    render_method == RENDER_WIRE ||
    render_method == RENDER_WIRE_VERTEX ||
    render_method == RENDERED_TEXTURED_WIRE ||
    render_method == RENDER_FILL_TRIANGLE_WIRE
  );
}

bool should_render_vertex(void) {
 return render_method == RENDER_WIRE_VERTEX;
}

// Initialize SDL Window and Renderer
bool initialize_window(void) {
  int is_SDL_initialized = SDL_Init(SDL_INIT_EVERYTHING);

  if (is_SDL_initialized != 0) {
    fprintf(stderr, "Error initializing SDL.\n");
    return false;
  }

  // Use SDL to query what is the fullscreen max. width and height
  SDL_DisplayMode display_mode;
  SDL_GetCurrentDisplayMode(0, &display_mode);

  int full_screen_window_width = display_mode.w;
  int full_screen_window_height = display_mode.h;

  window_width = display_mode.w;
  window_height = display_mode.h;

  // Create a SDL Window at center of the screen with 800 x 600 resolution with
  // always shown on top all application.
  window = SDL_CreateWindow(
    "Tiny3D",
    SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
    full_screen_window_width, full_screen_window_height,
    SDL_WINDOW_BORDERLESS
  );

  if (window == NULL) {
    fprintf(stderr, "Error creating SDL Window. \n");
    return false;
  }

  // Create a SDL Renderer to show inside of SDL Window with default options.
  renderer = SDL_CreateRenderer(window, -1, 0);

  if (renderer == NULL) {
    fprintf(stderr, "Error creating SDL Renderer. \n");
    return false;
  }

  SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);

  // Allocate the required memory in bytes to hold the color buffer and z buffer.
  color_buffer = (uint32_t*)malloc(sizeof(uint32_t) * window_width * window_height);
  z_buffer = (float*)malloc(sizeof(float) * window_width * window_height);

  if (color_buffer == NULL) {
    fprintf(stderr, "Error allocating memory to color_buffer. \n");
    return false;
  }

  // Creating a SDL texture that is used to display the color
  color_buffer_texture = SDL_CreateTexture(
    renderer,
    SDL_PIXELFORMAT_RGBA32,
    SDL_TEXTUREACCESS_STREAMING,
    window_width, window_height
  );
  
  return true;
}

void destroy_window(void) {
  free(z_buffer);
  free(color_buffer);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
}

void draw_grid(void) {
  for (int y = 0; y < window_height; y++) {
    for (int x = 0; x < window_width; x++) {
      if (x % 20 == 0 || y % 20 == 0) {
        color_buffer[(window_width * y) + x] = 0xFF333333;
      }
    }
  }
}

void draw_dots(void) {
  for (int y = 0; y < window_height; y = y + 20) {
    for (int x = 0; x < window_width; x = x + 20) {
      color_buffer[(window_width * y) + x] = 0xFF333333;
    }
  }
}

void draw_line(int x0, int y0, int x1, int y1, uint32_t color) {
  int delta_x = x1 - x0;
  int delta_y = y1 - y0;

  int side_length = abs(delta_x) >= abs(delta_y) ? abs(delta_x) : abs(delta_y);

  float x_inc = delta_x / (float)side_length;
  float y_inc = delta_y / (float)side_length;

  float current_x = x0;
  float current_y = y0;

  for (int i = 0; i <= side_length; i++) {
    draw_pixel(round(current_x), round(current_y), color);
    current_x += x_inc;
    current_y += y_inc;
  }
}

void draw_pixel(int x, int y, uint32_t color) {
  if (x >= 0 && y >= 0 && x < window_width && y < window_height) {
    color_buffer[(window_width * y) + x] = color;
  }
}

void draw_rect(int x, int y, int width, int height, uint32_t color) {
  for (int i = 0; i < width; i++) {
    for (int j = 0; j < height; j++) {
      int current_x = x + i;
      int current_y = y + j;
      draw_pixel(current_x, current_y, color);
    }
  }
}

void render_color_buffer(void) {
  SDL_UpdateTexture(
    color_buffer_texture,
    NULL,
    color_buffer,
    (int)(sizeof(uint32_t) * window_width)
  );
  SDL_RenderCopy(renderer, color_buffer_texture, NULL, NULL);
  SDL_RenderPresent(renderer);
}

float get_zbuffer_at(int x, int y) {
  if (x >= 0 && y >= 0 && x < window_width && y < window_height) {
    return z_buffer[(window_width * y) + x];
  }
  return 1.0;
}

void set_zbuffer_at(int x, int y, float v) {
  if (x >= 0 && y >= 0 && x < window_width && y < window_height) {
    z_buffer[(window_width * y) + x] = v;
  }
}

void clear_color_buffer(uint32_t color) {
  for (int i = 0; i < window_width * window_height; i++) {
      color_buffer[i] = color;
  }
}

void clear_z_buffer(void) {
  for (int i = 0; i < window_width * window_height; i++) {
      z_buffer[i] = 1.0;
  }
}