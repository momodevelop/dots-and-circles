#ifndef PLATFORM_SDL
#define PLATFORM_SDL

#include "thirdparty/sdl2/include/SDL.h"
#include "game_renderer.h"
#include "mm_core.h"


struct sdl_context {
    SDL_Window* Window;
    union {
        SDL_GLContext GlContext;
    };
    renderer* Renderer;
};

typedef option<sdl_context> sdl_load(u32 WindowWidth, u32 WindowHeight);
typedef void sdl_resize(renderer* Renderer, u32 Width, u32 Height);
typedef void sdl_render(renderer* Renderer, mailbox* Commands);
typedef void sdl_unload(sdl_context Context);
typedef void sdl_swap_buffer(sdl_context Context);
struct sdl_api {
    sdl_load* Load;
    sdl_resize* Resize;
    sdl_render* Render;
    sdl_unload* Unload;
    sdl_swap_buffer* SwapBuffer;
};

#endif
