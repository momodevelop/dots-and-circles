#ifndef __YUU_UTILS_H__
#define __YUU_UTILS_H__

#include <optional>
#include "sdl2/SDL.h"

namespace yuu {


	std::pair<int, int> getWindowSize(SDL_Window* window);
	SDL_Event* pollEvent();

}

#endif