#include <stdbool.h>
#include <stdio.h>

#include <SDL.h>

int main(int argc, char *argv[])
{

    bool isquit = false;
    SDL_Event event;

    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("Failed to initialize the SDL2 library\n");
        return -1;
    }

    SDL_Window *window = SDL_CreateWindow(
        "SDL2 Window",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        680, 480,
        0);

    if (!window)
    {
        printf("Failed to create window\n");
        return -1;
    }

    SDL_Surface *window_surface = SDL_GetWindowSurface(window);

    if (!window_surface)
    {
        printf("Failed to get the surface from the window\n");
        return -1;
    }

    SDL_UpdateWindowSurface(window);

    while (!isquit)
    {
        if (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                isquit = true;
            }
        }
    }

    return 0;
}
