#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>

#include "display.h"
#include "vector.h"
#include "mesh.h"

triangle_t triangle_to_render[N_MESH_FACES];

vec3_t camera_position = {.x = 0, .y = 0, .z = -5};
vec3_t cube_rotation = {.x = 0, .y = 0, .z = 0};

float fov_factor = 640;

bool is_paused = false;
bool is_running = false;

int previous_time_frame = 0;

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

        if (event.key.keysym.sym == SDLK_SPACE)
            is_paused = !is_paused;

        break;
    }
}

vec2_t project(vec3_t point)
{
    vec2_t projected_point = {
        .x = ((fov_factor * point.x) / point.z),
        .y = ((fov_factor * point.y) / point.z),
    };
    return projected_point;
}

void update(void)
{
    int time_to_wait = FRAME_TARGET_TIME - (SDL_GetTicks64() - previous_time_frame);

    if (time_to_wait > 0 && time_to_wait <= FRAME_TARGET_TIME)
    {
        SDL_Delay(time_to_wait);
    }

    previous_time_frame = SDL_GetTicks64();

    if (is_paused == false)
    {
        cube_rotation.y += 0.01;
    }

    for (int i = 0; i < N_MESH_FACES; i++)
    {
        face_t mesh_face = mesh_faces[i];

        // Due to index of array starts with 0, descreasing mesh_face index with -1
        vec3_t face_vertices[3] = {
            mesh_vertices[mesh_face.a - 1],
            mesh_vertices[mesh_face.b - 1],
            mesh_vertices[mesh_face.c - 1],
        };

        triangle_t projected_triangle;

        // Loop all three vertices of this current face and apply transformations
        for (int j = 0; j < 3; j++)
        {
            vec3_t transformed_vertex = vec3_rotate_x(face_vertices[j], cube_rotation.y);

            // Move the point away from the camera
            transformed_vertex.z -= camera_position.z;

            // Project the current vertex
            vec2_t projected_vertex = project(transformed_vertex);

            // Translate the vertex to the middle of the screen
            projected_vertex.x += (window_width / 2);
            projected_vertex.y += (window_height / 2);

            projected_triangle.points[j] = projected_vertex;
        }

        // Save the projected triangle in the array of triangle to render
        triangle_to_render[i] = projected_triangle;
    }
}

void render(void)
{
    draw_dots();

    // Loop all projected triangle and render then
    for (int i = 0; i < N_MESH_FACES; i++)
    {
        triangle_t triangle = triangle_to_render[i];
        draw_triangle(triangle.points[0].x, triangle.points[0].y, triangle.points[1].x, triangle.points[1].y, triangle.points[2].x, triangle.points[2].y, 0xFFFFFF00);
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
