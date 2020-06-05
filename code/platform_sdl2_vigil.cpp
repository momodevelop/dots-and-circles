#include <stdio.h>
#include "ryoji_common.cpp"
#include "yuu_include_glad.cpp"
#include "yuu_include_sdl2.cpp"

//#include "vigil_interface.h"

global bool gIsRunning = true;


void GameLog(const char * str, ...) {
    va_list va;
    va_start(va, str);
    SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_VERBOSE, str, va);
    va_end(va);
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
    SDL_Window* window = SDL_CreateWindow("Vigil", 
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
        SDL_Log("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }
    
    
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    
    
    // Request an OpenGL 4.5 context (should be core)
    SDL_GLContext context = SDL_GL_CreateContext(window);
    if (context == nullptr) { 
        SDL_Log("Failed to create OpenGL context");
    }
    
    
    // TODO(Momo): Game state init here
    
    SDLTimer timer;
    SDLTimerStart(&timer);
    
    while(gIsRunning) {
        SDL_Event e;
        while(SDL_PollEvent(&e)) {
            switch(e.type) {
                case SDL_QUIT: {
                    gIsRunning = false;
                    SDL_Log("Quit triggered\n");
                } break; 
            }
        }
        
        
        // TODO(Momo): Update + Render
        SDLTimerTick(&timer);
        SDL_Log("%lld  ms\n", SDLTimerGetTimeElapsed(&timer));
    }
    
    
    return 0;
    
}