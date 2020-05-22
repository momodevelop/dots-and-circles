#ifndef __YUU_UTILS_H__
#define __YUU_UTILS_H__

#include <optional>
#include "sdl2/SDL.h"
#include "ryoji/maybe.h"
namespace yuu {
	struct WindowSize { int w, h; } getWindowSize(SDL_Window* window);
	ryoji::Maybe<SDL_Event> pollEvent();

}

#endif