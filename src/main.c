#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>

#include "display.h"
#include "vector.h"

#define N_POINTS (9 * 9 * 9)

vec3_t cube_points[N_POINTS];
vec2_t projected_points[N_POINTS];

vec3_t camera_position = {0, 0, -5};

float fov_factor = 640;

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

    int point_count = 0;

    for (float x = -1; x <= 1; x += 0.25)
    {
        for (float y = -1; y <= 1; y += 0.25)
        {
            for (float z = -1; z <= 1; z += 0.25)
            {
                vec3_t new_cube_point = {x, y, z};
                cube_points[point_count++] = new_cube_point;
            }
        }
    }

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

vec2_t project(vec3_t point)
{
    vec2_t projected_point = {
        .x = ((fov_factor * point.x) / point.z) + window_width / 2,
        .y = ((fov_factor * point.y) / point.z) + window_height / 2,
    };
    return projected_point;
}

void update(void)
{
    for (int i = 0; i < N_POINTS; i++)
    {
        vec3_t point = cube_points[i];

        // Move the points away from the camera
        point.z -= camera_position.z;

        // Project the current point
        vec2_t projected_point = project(point);

        // Save the projected 2D vector in the array of projected points
        projected_points[i] = projected_point;
    }
}

void render(void)
{
    draw_dots();

    // Loop all projected points and render then
    for (int i = 0; i < N_POINTS; i++)
    {
        vec2_t projected_point = projected_points[i];
        draw_rect(projected_point.x, projected_point.y, 4, 4, 0xFFFFFF00);
    }

    render_color_buffer();

    clear_color_buffer(0xFF000000);

    SDL_RenderPresent(renderer);
}

int main(int argc, char *argv[])
{
    is_running = initialize_window();

    setup();

    while (is_running)
    {
        process_input();
        update();
        render();
    }

    destroy_window();

    return 0;
}
