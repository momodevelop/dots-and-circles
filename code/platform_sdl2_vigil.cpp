#include "SDL.h"
#include "ryoji_common.cpp"

int main(int argc, char* argv[]) {
    
    (void)argc;
    (void)argv;
    printf("SDL initializing\n");
    if (SDL_Init(SDL_INIT_VIDEO) < 0 ) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }
    defer{
        printf("SDL shutting down\n");
        SDL_Quit();
    };
    
    printf("SDL creating Window\n");
    auto* window = SDL_CreateWindow("Vigil", 
                                    SDL_WINDOWPOS_UNDEFINED, 
                                    SDL_WINDOWPOS_UNDEFINED, 
                                    1600, 
                                    900, 
                                    SDL_WINDOW_SHOWN);
    defer{
        printf("SDL destroying window\n");
        SDL_DestroyWindow(window);
    };
    
    if (window == nullptr) {
        printf("Window could not be create! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }
    
    SDL_Delay(2000);
    
    return 0;
    
} 