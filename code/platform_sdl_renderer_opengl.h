#ifndef PLATFORM_SDL_RENDERER_OPENGL
#define PLATFORM_SDL_RENDERER_OPENGL

#include <stdlib.h>
#include "game_renderer_opengl.h"
#include "platform_sdl_renderer.h"
#include "mm_mailbox.h"
#include "mm_arena.h"

static inline renderer*
SdlOpenglLoad(SDL_Window* Window) {
    i32 Width, Height;
    SDL_GetWindowSize(Window, &Width, &Height);

    // TODO: More initialization here 
    renderer_opengl* Ret = (renderer_opengl*)malloc(sizeof(renderer_opengl));
    Init(Ret, Width, Height, 10000);

    return (renderer*)Ret;
}

static inline void
SdlOpenglUnload(renderer* Renderer) {
    free(Renderer);
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


#endif
