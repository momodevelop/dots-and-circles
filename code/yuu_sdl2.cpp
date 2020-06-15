#ifndef __YUU_SDL2__
#define __YUU_SDL2__

#include "thirdparty/sdl2/include/SDL.h"

enum SDLError {
    SDLERR_NONE,
    SDLERR_FILE_CANNOT_OPEN,
    SDLERR_DEST_TOO_SMOL,
};

pure const char* SDLErrorGetMsg(SDLError error) {
    switch (error){
        case SDLERR_FILE_CANNOT_OPEN: {
            return "Cannot open file";
        }
        case SDLERR_DEST_TOO_SMOL: {
            return "Destination too smol";
        }
        
    }
    return nullptr;
}

// NOTE(Momo): SDL Timer
struct SDLTimer {
    u64 countFrequency;
    u64 prevFrameCounter;
    u64 endFrameCounter;
    u64 countsElapsed;
};
pure void Start(SDLTimer* timer) {
    timer->countFrequency = SDL_GetPerformanceFrequency();
    timer->prevFrameCounter = SDL_GetPerformanceCounter();
    timer->endFrameCounter = 0;
    timer->countsElapsed = 0;
}
pure void Tick(SDLTimer * timer) {
    timer->endFrameCounter = SDL_GetPerformanceCounter();
    timer->countsElapsed = timer->endFrameCounter - timer->prevFrameCounter;
    timer->prevFrameCounter = timer->endFrameCounter; 
}

pure u64 TimeElapsed(SDLTimer * timer) {
    // NOTE(Momo): 
    // PerformanceCounter(C) gives how many count has elapsed.
    // PerformanceFrequency(F) gives how many counts/second.
    // Thus: seconds = C / F, and milliseconds = seconds * 1000
    return (1000 * timer->countsElapsed) / timer->countFrequency;
}

// NOTE(Momo): Helpers
struct SDLWindowSize{ i32 w, h; } SDLGetWindowSize(SDL_Window* window) {
    int w, h;
    SDL_GetWindowSize(window, &w, &h);
    return { w, h };
}

pure SDLError SDLReadFileStr(char* dest, Sint64 destSize, const char * path) {
    SDL_RWops * file = SDL_RWFromFile(path, "r");
    if (file == nullptr) {
        return SDLERR_FILE_CANNOT_OPEN;
    }
    Defer{
        SDL_RWclose(file);
    };
    
    // Get file size
    Sint64 filesize = SDL_RWsize(file); // Does not include EOF
    if ((filesize + 1) > destSize) {
        return SDLERR_DEST_TOO_SMOL;
    }
    
    SDL_RWread(file, dest, sizeof(char), filesize);
    
    // Don't forget null terminating value
    dest[filesize] = 0;
    
    return SDLERR_NONE;
}

pure SDLError SDLReadFileBin(void* dest, Sint64 destSize, const char * path) {
    SDL_RWops * file = SDL_RWFromFile(path, "rb");
    if (file == nullptr) {
        return SDLERR_FILE_CANNOT_OPEN;
    }
    Defer{
        SDL_RWclose(file);
    };
    
    // Get file size
    Sint64 filesize = SDL_RWsize(file); // Does not include EOF
    if (filesize > destSize) {
        return SDLERR_DEST_TOO_SMOL;
    }
    SDL_RWread(file, dest, sizeof(char), filesize);
    return SDLERR_NONE;
}



#endif