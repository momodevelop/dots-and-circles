#include <stdio.h>
#include "SDL.h"
#include "ryoji_common.cpp"
//#include "vigil_interface.h"

global bool gIsRunning = true;


struct SDLTimer {
    u64 countFrequency;
    u64 prevFrameCounter;
    u64 endFrameCounter;
    u64 countsElapsed;
};
void SDLTimerStart(SDLTimer* timer) {
    timer->countFrequency = SDL_GetPerformanceFrequency();
    timer->prevFrameCounter = SDL_GetPerformanceCounter();
    timer->endFrameCounter = 0;
    timer->countsElapsed = 0;
}
void SDLTimerTick(SDLTimer * timer) {
    timer->endFrameCounter = SDL_GetPerformanceCounter();
    timer->countsElapsed = timer->endFrameCounter - timer->prevFrameCounter;
    timer->prevFrameCounter = timer->endFrameCounter; 
    
}
u64 SDLTimerGetTimeElapsed(SDLTimer * timer) {
    // NOTE(Momo): Quick tip 
    // PerformanceCounter(C) gives how many count has elapsed.
    // PerformanceFrequency(F) gives how many counts/second.
    // Thus: seconds = C / F, and milliseconds = seconds * 1000
    return (1000 * timer->countsElapsed) / timer->countFrequency;
}




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
    
    
    SDLTimer timer;
    SDLTimerStart(&timer);
    
    while(gIsRunning) {
        // TODO(Momo): Poll Event
        // TODO(Momo): Update + Render
        
        
        
        SDLTimerTick(&timer);
        printf("%lld  ms\n", SDLTimerGetTimeElapsed(&timer));
        
        
        
        
        SDL_Delay(2000);
    }
    
    
    
    
    
    
    
    
    
    
    SDL_Delay(2000);
    
    return 0;
    
} 