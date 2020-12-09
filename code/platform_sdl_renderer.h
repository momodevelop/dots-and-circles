#ifndef PLATFORM_SDL_RENDERER
#define PLATFORM_SDL_RENDERER

#include "thirdparty/sdl2/include/SDL.h"
#include "game_renderer.h"
#include "mm_core.h"


struct sdl_renderer_context {
    SDL_Window* Window;
    union {
        SDL_GLContext GlContext;
    };
    renderer* Renderer;
};

typedef option<sdl_renderer_context> sdl_renderer_load(u32 WindowWidth, u32 WindowHeight);
typedef void renderer_resize(renderer* Renderer, u32 Width, u32 Height);
typedef void renderer_render(renderer* Renderer, mailbox* Commands);
typedef void sdl_renderer_unload(sdl_renderer_context Renderer);
struct sdl_renderer_api {
    sdl_renderer_load* Load;
    renderer_resize* Resize;
    renderer_render* Render;
    sdl_renderer_unload* Unload;
};

#endif
