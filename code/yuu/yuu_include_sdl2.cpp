#include "sdl2/include/SDL.h"


// NOTE(Momo): SDL Timer
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
