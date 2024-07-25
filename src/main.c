#include <stdio.h>
#include <stdbool.h>
#include <SDL2/SDL.h>

bool is_running = false;

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;

// Initialize SDL Window and Renderer
bool initialize_window(void)
{
    int is_SDL_initialized = SDL_Init(SDL_INIT_EVERYTHING);

    if (is_SDL_initialized != 0)
    {
        fprintf(stderr, "Error initializing SDL.\n");
        return false;
    }

    // Create a SDL Window at center of the screen with 800 x 600 resolution with always shown on top all application.
    window = SDL_CreateWindow("Tiny3D", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_ALWAYS_ON_TOP);

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

    return true;
}

void setup(void) {}

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
    // Set the drawing color
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);

    // Clears or fills the background with the current drawing color set via SDL_SetRenderDrawColor.
    SDL_RenderClear(renderer);

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

    return 0;
}
