#ifndef __SDL_PLATFORM_UTILS__
#define __SDL_PLATFORM_UTILS__

struct sdl_window_size {
    i32 Width, Height;
};

static inline sdl_window_size 
SDLGetWindowSize(SDL_Window* Window) {
    i32 w, h;
    SDL_GetWindowSize(Window, &w, &h);
    return { w, h };
}

#endif