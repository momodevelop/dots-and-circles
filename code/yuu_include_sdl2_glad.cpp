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

pure void GLDebugInit(GLDebug* debugObj, void (*logger)(const char*,...)) {
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    
    debugObj->logger = logger;
    
    auto callback = [](GLenum source,
                       GLenum type,
                       GLuint id,
                       GLenum severity,
                       GLsizei length,
                       const GLchar* msg,
                       const void* userParam)
    {
        (void)length; 
        (void)userParam;
        char* _source;
        char* _type;
        char* _severity;
        switch (source) {
            case GL_DEBUG_SOURCE_API:
            _source = "API";
            break;
            
            case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
            _source = "WINDOW SYSTEM";
            break;
            
            case GL_DEBUG_SOURCE_SHADER_COMPILER:
            _source = "SHADER COMPILER";
            break;
            
            case GL_DEBUG_SOURCE_THIRD_PARTY:
            _source = "THIRD PARTY";
            break;
            
            case GL_DEBUG_SOURCE_APPLICATION:
            _source = "APPLICATION";
            break;
            
            case GL_DEBUG_SOURCE_OTHER:
            _source = "UNKNOWN";
            break;
            
            default:
            _source = "UNKNOWN";
            break;
        }
        
        switch (type) {
            case GL_DEBUG_TYPE_ERROR:
            _type = "ERROR";
            break;
            
            case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
            _type = "DEPRECATED BEHAVIOR";
            break;
            
            case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
            _type = "UDEFINED BEHAVIOR";
            break;
            
            case GL_DEBUG_TYPE_PORTABILITY:
            _type = "PORTABILITY";
            break;
            
            case GL_DEBUG_TYPE_PERFORMANCE:
            _type = "PERFORMANCE";
            break;
            
            case GL_DEBUG_TYPE_OTHER:
            _type = "OTHER";
            break;
            
            case GL_DEBUG_TYPE_MARKER:
            _type = "MARKER";
            break;
            
            default:
            _type = "UNKNOWN";
            break;
        }
        
        switch (severity) {
            case GL_DEBUG_SEVERITY_HIGH:
            _severity = "HIGH";
            break;
            
            case GL_DEBUG_SEVERITY_MEDIUM:
            _severity = "MEDIUM";
            break;
            
            case GL_DEBUG_SEVERITY_LOW:
            _severity = "LOW";
            break;
            
            case GL_DEBUG_SEVERITY_NOTIFICATION:
            _severity = "NOTIFICATION";
            break;
            
            default:
            _severity = "UNKNOWN";
            break;
        }
        //printf("%s\n", msg);
        reinterpret_cast<const GLDebug*>(userParam)->logger("%d: %s of %s severity, raised from %s: %s\n",
                                                            id, _type, _severity, _source, msg);
        
    };
    
    glDebugMessageCallback(callback, debugObj);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, true);
}
