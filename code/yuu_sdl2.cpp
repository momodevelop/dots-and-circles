#ifndef __YUU_SDL2__
#define __YUU_SDL2__

#include "thirdparty/sdl2/include/SDL.h"


// NOTE(Momo): SDL Timer
struct SDLTimer {
    u64 CountFrequency;
    u64 PrevFrameCounter;
    u64 EndFrameCounter;
    u64 CountsElapsed;
};

pure 
void 
Start(SDLTimer* timer) {
    timer->CountFrequency = SDL_GetPerformanceFrequency();
    timer->PrevFrameCounter = SDL_GetPerformanceCounter();
    timer->EndFrameCounter = 0;
    timer->CountsElapsed = 0;
}


pure void Tick(SDLTimer * timer) {
    timer->EndFrameCounter = SDL_GetPerformanceCounter();
    timer->CountsElapsed = timer->EndFrameCounter - timer->PrevFrameCounter;
    
    timer->PrevFrameCounter = timer->EndFrameCounter; 
}

pure u64 TimeElapsed(SDLTimer * timer) {
    // NOTE(Momo): 
    // PerformanceCounter(C) gives how many count has elapsed.
    // PerformanceFrequency(F) gives how many counts/second.
    // Thus: seconds = C / F, and milliseconds = seconds * 1000
    return (1000 * timer->CountsElapsed) / timer->CountFrequency;
}

// NOTE(Momo): Helpers
struct SDLWindowSize { i32 Width, Height; };

pure SDLWindowSize SDLGetWindowSize(SDL_Window* window) {
    i32 w, h;
    SDL_GetWindowSize(window, &w, &h);
    return { w, h };
}



#endif