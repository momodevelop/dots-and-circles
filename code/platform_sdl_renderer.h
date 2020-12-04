#ifndef PLATFORM_SDL_RENDERER
#define PLATFORM_SDL_RENDERER

#include "thirdparty/sdl2/include/SDL.h"
#include "game_renderer.h"
#include "mm_core.h"

typedef renderer* renderer_load(SDL_Window* Window);
typedef void renderer_resize(renderer* Renderer, u32 Width, u32 Height);
typedef void renderer_render(renderer* Renderer, mailbox* Commands);
typedef void renderer_unload(renderer* Renderer);
struct renderer_api {
    renderer_load* Load;
    renderer_resize* Resize;
    renderer_render* Render;
    renderer_unload* Unload;
};

#endif
