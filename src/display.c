#include <stdbool.h>
#include <SDL2/SDL.h>

#include "display.h"

int window_width = 800;
int window_height = 600;

uint32_t *color_buffer = NULL;

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
SDL_Texture *color_buffer_texture = NULL;

// Initialize SDL Window and Renderer
bool initialize_window(void)
{
    int is_SDL_initialized = SDL_Init(SDL_INIT_EVERYTHING);

    if (is_SDL_initialized != 0)
    {
        fprintf(stderr, "Error initializing SDL.\n");
        return false;
    }

    // Use SDL to query what is the fullscreen max. width and height
    SDL_DisplayMode display_mode;
    SDL_GetCurrentDisplayMode(0, &display_mode);

    window_width = display_mode.w;
    window_height = display_mode.h;

    // Create a SDL Window at center of the screen with 800 x 600 resolution with always shown on top all application.
    window = SDL_CreateWindow("Tiny3D", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, window_width, window_height, SDL_WINDOW_ALWAYS_ON_TOP);

    if (window == NULL)
    {
        fprintf(stderr, "Error creating SDL Window. \n");
        return false;
    }

    // Create a SDL Renderer to show inside of SDL Window with default options.
    renderer = SDL_CreateRenderer(window, -1, 0);

    if (renderer == NULL)
    {
        fprintf(stderr, "Error creating SDL Renderer. \n");
        return false;
    }

    SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);

    return true;
}

void destroy_window(void)
{
    free(color_buffer);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void draw_grid(void)
{
    for (int y = 0; y < window_height; y++)
    {
        for (int x = 0; x < window_width; x++)
        {
            if (x % 20 == 0 || y % 20 == 0)
            {
                color_buffer[(window_width * y) + x] = 0xFF333333;
            }
        }
    }
}

void draw_dots(void)
{
    for (int y = 0; y < window_height; y = y + 20)
    {
        for (int x = 0; x < window_width; x = x + 20)
        {
            color_buffer[(window_width * y) + x] = 0xFF333333;
        }
    }
}

void draw_pixel(int x, int y, uint32_t color)
{
    if (x >= 0 && y >= 0 && x < window_width && y < window_height)
    {
        color_buffer[(window_width * y) + x] = color;
    }
}

void draw_rect(int x, int y, int width, int height, uint32_t color)
{
    for (int i = 0; i < width; i++)
    {
        for (int j = 0; j < height; j++)
        {
            int current_x = x + i;
            int current_y = y + j;
            draw_pixel(current_x, current_y, color);
        }
    }
}

void render_color_buffer(void)
{
    SDL_UpdateTexture(color_buffer_texture, NULL, color_buffer, (int)(sizeof(u_int32_t) * window_width));
    SDL_RenderCopy(renderer, color_buffer_texture, NULL, NULL);
}

void clear_color_buffer(uint32_t color)
{
    for (int y = 0; y < window_height; y++)
    {
        for (int x = 0; x < window_width; x++)
        {
            color_buffer[(window_width * y) + x] = color;
        }
    }
}