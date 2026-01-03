#include <stdio.h>
#include <SDL2/SDL.h>

int main(int argc, char *argv[])
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("SDL_Init Error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window *win = SDL_CreateWindow("Key Test", 
                                       SDL_WINDOWPOS_CENTERED, 
                                       SDL_WINDOWPOS_CENTERED, 
                                       200, 200, 0);
    if (!win) {
        printf("SDL_CreateWindow Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    int running = 1;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    running = 0;
                    break;
                case SDL_KEYDOWN:
                    printf("Key DOWN: %s\n", SDL_GetKeyName(event.key.keysym.sym));
                    break;
                case SDL_KEYUP:
                    printf("Key UP: %s\n", SDL_GetKeyName(event.key.keysym.sym));
                    break;
            }
        }
        SDL_Delay(10);
    }

    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}