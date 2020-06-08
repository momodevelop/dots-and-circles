#include "thirdparty/sdl2/include/SDL.h"
#include "thirdparty/glad/glad.c"
// NOTE(Momo): Errors
enum struct SDLError {
    NONE,
    FILE_CANNOT_OPEN,
    FILESIZE_BIGGER_THAN_BUFFER,
    FILESIZE_AND_READSIZE_DUN_MATCH,
};

pure const char* SDLErrorGetMsg(SDLError error) {
    switch (error){
        case SDLError::FILE_CANNOT_OPEN: {
            return "Cannot open file";
        }
        case SDLError::FILESIZE_BIGGER_THAN_BUFFER: {
            return "Filesize is bigger than buffer";
        }
        case SDLError::FILESIZE_AND_READSIZE_DUN_MATCH: {
            return "Filesize and readsize don't match";
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
pure void SDLTimerStart(SDLTimer* timer) {
    timer->countFrequency = SDL_GetPerformanceFrequency();
    timer->prevFrameCounter = SDL_GetPerformanceCounter();
    timer->endFrameCounter = 0;
    timer->countsElapsed = 0;
}
pure void SDLTimerTick(SDLTimer * timer) {
    timer->endFrameCounter = SDL_GetPerformanceCounter();
    timer->countsElapsed = timer->endFrameCounter - timer->prevFrameCounter;
    timer->prevFrameCounter = timer->endFrameCounter; 
}

pure u64 SDLTimerGetTimeElapsed(SDLTimer * timer) {
    // NOTE(Momo): Quick tip 
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


// NOTE(Momo): File IO
pure SDLError SDLReadFileToString(char* dest, Sint64 destSize, const char * path, bool binary) {
    SDL_RWops * file = SDL_RWFromFile(path, binary ? "rb" : "r");
    if (file == nullptr) {
        return SDLError::FILE_CANNOT_OPEN;
    }
    Defer{
        SDL_RWclose(file);
    };
    
    // Get file size
    Sint64 filesize = SDL_RWsize(file);
    if ((filesize + 1) > destSize) {
        return SDLError::FILESIZE_BIGGER_THAN_BUFFER;
    }
    
    // Read into the buffer
    char* itr = dest;
    Sint64 readTotal = 0;
    
    
    while(readTotal < filesize) {
        size_t readAmount = SDL_RWread(file, itr, sizeof(char), 1);
        readTotal += readAmount;
        itr += readAmount;
        
    }
    
    // If the total amount of bytes read is not the filesize, 
    // something went wrong although honestly, the damage was done lol
    if (readTotal < filesize) {
        return SDLError::FILESIZE_AND_READSIZE_DUN_MATCH;
    }
    
    // Don't forget null terminating value
    itr[readTotal] = 0;
    return SDLError::NONE;
}

// NOTE(Momo): GL-related

struct GLDebug {
    typedef void (*LoggerType)(const char* fmt, ...);
    LoggerType logger;
};

pure GLDebugInit(GLDebug * debugObj, GLDebug::LoggerType logger) {
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    
    loggerWrapper { logger };
    
    auto callback = [](GLenum source,
                       GLenum type,
                       GLuint id,
                       GLenum severity,
                       GLsizei length,
                       const GLchar* msg,
                       const void* userParam){
        
        (void)severity;
        (void)id;
        (void)type;
        (void)source;
        (void)length; 
        (void)userParam;
        //printf("%s\n", msg);
        reinterpret_cast<const GLDebug*>(userParam)->logger("%s\n", msg);;
        
    };
    
    glDebugMessageCallback(callback, debugObj);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, true);
}
