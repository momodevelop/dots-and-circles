#ifndef PLATFORM_SDL_OPENGL
#define PLATFORM_SDL_OPENGL

#include <stdlib.h>
#include "thirdparty/sdl2/include/SDL.h"
#include "thirdparty/glad/glad.c"
#include "game_renderer_opengl.h"
#include "platform_sdl.h"
#include "mm_mailbox.h"
#include "mm_arena.h"


#if INTERNAL
static inline void
SdlGlDebugCallback(GLenum source,
                   GLenum type,
                   GLuint id,
                   GLenum severity,
                   GLsizei length,
                   const GLchar* msg,
                   const void* userParam) {
    // Ignore NOTIFICATION severity
    if (severity == GL_DEBUG_SEVERITY_NOTIFICATION) 
        return;
    
    const char* _source;
    const char* _type;
    const char* _severity;
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
        //_severity = "LOW";
        return;
        
        case GL_DEBUG_SEVERITY_NOTIFICATION:
        //_severity = "NOTIFICATION";
        return;
        
        default:
        _severity = "UNKNOWN";
        break;
    }
    
    SDL_Log("[OpenGL] %d: %s of %s severity, raised from %s: %s\n",
            id, _type, _severity, _source, msg);
    
};
#endif

static inline option<sdl_context>
SdlOpenglLoad(u32 WindowWidth, u32 WindowHeight) {
    sdl_context Ret = {};
   
    // NOTE(Momo): Create Window
    SDL_GL_LoadLibrary(nullptr);
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
#if INTERNAL
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
#endif  

    SDL_Log("SDL creating OpenGL window");
    SDL_Window* Window = SDL_CreateWindow("Vigil", 
                                          SDL_WINDOWPOS_UNDEFINED, 
                                          SDL_WINDOWPOS_UNDEFINED, 
                                          WindowWidth, 
                                          WindowHeight, 
                                          SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
   
    
    if (Window == nullptr) {
        SDL_Log("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return None<sdl_context>();
    }

    SDL_Log("SDL creating Opengl context\n");
    SDL_GLContext Context = SDL_GL_CreateContext(Window);
    
    if (Context == nullptr) { 
        SDL_Log("Failed to create OpenGL context! SDL_Error: %s\n", SDL_GetError());
        return None<sdl_context>();
    }
    gladLoadGLLoader(SDL_GL_GetProcAddress);

    // VSync
    SDL_Log("[OpenGL] Loaded!\n");
    if (SDL_GL_SetSwapInterval(-1) < 0) {
        if (SDL_GL_SetSwapInterval(1) < 0) {
            SDL_GL_SetSwapInterval(0);
            SDL_Log("[OpenGL] No VSync Enabled");
        }
        else {
            SDL_Log("[OpenGL] Normal VSync Enabled");
        }
    }
    else {
        SDL_Log("[OpenGL] Adaptive VSync Enabled");
    }


    SDL_Log("[OpenGL] Vendor:   %s\n", glGetString(GL_VENDOR));
    SDL_Log("[OpenGL] Renderer: %s\n", glGetString(GL_RENDERER));
    SDL_Log("[OpenGL] Version:  %s\n", glGetString(GL_VERSION));
    
#ifdef INTERNAL
    glDebugMessageCallback(SdlGlDebugCallback, nullptr);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, true);
#endif
    

    
    i32 Width, Height;
    SDL_GetWindowSize(Window, &Width, &Height);

    // TODO: More initialization here 
    renderer_opengl* Renderer = (renderer_opengl*)malloc(sizeof(renderer_opengl));
    Init(Renderer, Width, Height, 10000);
    

    Ret.Renderer = (renderer*)Renderer;
    Ret.Window = Window;
    Ret.GlContext = Context;
    


    return Some(Ret);
}

static inline void
SdlOpenglUnload(sdl_context Context) {
    free(Context.Renderer);
    SDL_GL_DeleteContext(Context.GlContext);
    SDL_DestroyWindow(Context.Window);
    SDL_Quit();
}


static inline void
SdlOpenglResize(renderer* Renderer, u32 Width, u32 Height) {
    OpenglResize((renderer_opengl*)Renderer, Width, Height);
}


static inline void
SdlOpenglRender(renderer* Renderer, mailbox* Commands) {
    OpenglRender((renderer_opengl*)Renderer, Commands);
    Clear(Commands);
}

static inline void 
SdlOpenglSwapBuffer(sdl_context Context) {
    SDL_GL_SwapWindow(Context.Window);
}

#endif
