#include <stdio.h>
#include "SDL.h"
#include "ryoji_common.cpp"
//#include "vigil_interface.h"

global bool gIsRunning = true;

// TODO(Momo): Timer struct for SDL.

int main(int argc, char* argv[]) {
    
    // NOTE(Momo):  Initialization
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
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }
    
    
    
    u64 prevCounter = SDL_GetPerformanceCounter();
    u64 countFrequency = SDL_GetPerformanceFrequency();
    
    while(gIsRunning) {
        
        
        
        
        
        // TODO(Momo): Poll Event
        
        
        
        
        // TODO(Momo): Update + Render
        
        
        u64 endCounter = SDL_GetPerformanceCounter();
        u64 countsElapsed = endCounter - prevCounter;
        
        
        // NOTE(Momo): Quick tip 
        // PerformanceCounter(C) gives how many count has elapsed.
        // PerformanceFrequency(F) gives how many counts/second.
        // Thus: seconds = C / F, and milliseconds = seconds * 1000
        u64 msElapsed = (1000 * countsElapsed) / countFrequency;
        printf("%lld  ms\n", msElapsed);
        
        
        // TODO(Momo): Display the value?
        prevCounter = endCounter; 
        
        SDL_Delay(2000);
    }
    
    
    
    
    
    
    
    
    
    
    SDL_Delay(2000);
    
    return 0;
    
} 