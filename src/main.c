#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>

#include "display.h"

bool is_running = false;

bool setup(void)
{
    // Allocate the required memory in bytes to hold the color buffer.
    color_buffer = (uint32_t *)malloc(sizeof(uint32_t) * window_width * window_height);

    if (color_buffer == NULL)
    {
        fprintf(stderr, "Error allocating memory to color_buffer. \n");
        return false;
    }

    // Creating a SDL texture that is used to display the color
    color_buffer_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, window_width, window_height);

    return true;
}

void process_input(void)
{
    SDL_Event event;

    // Store the current event in the event variable to process system events (e.g., window close, resize) and keyboard input (e.g., key presses, releases).
    SDL_PollEvent(&event);

    switch (event.type)
    {
    // Triggered when the user attempts to close the application, such as clicking the window's close button or pressing Alt+F4.
    case SDL_QUIT:
        is_running = false;

        break;

    // Triggered when key is pressed on the keyboard.
    case SDL_KEYDOWN:
        // SDLK stands for SDL Keycode
        if (event.key.keysym.sym == SDLK_ESCAPE)
            is_running = false;

        break;
    }
}

void update(void) {}

void render(void)
{
    draw_dots();
    draw_rect(100, 100, 200, 200, 0xFFFFFF);
    draw_rect(300, 300, 200, 200, 0xFFFFFF);

    render_color_buffer();

    // Clearing the color buffer with specified color before next rendering.
    clear_color_buffer(0xFF000000);

    // Present the rendered content to the screen.
    SDL_RenderPresent(renderer);
}

int main(int argc, char *argv[])
{
    is_running = initialize_window();

    setup();

    // Game loop
    while (is_running)
    {
        process_input();
        update();
        render();
    }

    destroy_window();

    return 0;
}
