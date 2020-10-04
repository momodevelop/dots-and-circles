#ifndef __PLATFORM_SDL_TIMER__
#define __PLATFORM_SDL_TIMER__


struct sdl_timer {
    u64 CountFrequency;
    u64 PrevFrameCounter;
    u64 EndFrameCounter;
    u64 CountsElapsed;
};



static inline 
void Start(sdl_timer* Timer) {
    Timer->CountFrequency = SDL_GetPerformanceFrequency();
    Timer->PrevFrameCounter = SDL_GetPerformanceCounter();
    Timer->EndFrameCounter = 0;
    Timer->CountsElapsed = 0;
}


static inline 
u64 GetTicksElapsed(sdl_timer* Timer) {
    Timer->EndFrameCounter = SDL_GetPerformanceCounter();
    Timer->CountsElapsed = Timer->EndFrameCounter - Timer->PrevFrameCounter;
    
    Timer->PrevFrameCounter = Timer->EndFrameCounter; 
    
    // NOTE(Momo): 
    // PerformanceCounter(C) gives how many count has elapsed.
    // PerformanceFrequency(F) gives how many counts/second.
    // Thus: seconds = C / F, and milliseconds = seconds * 1000
    return (1000 * Timer->CountsElapsed) / Timer->CountFrequency;
}


#endif