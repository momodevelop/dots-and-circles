#ifndef __YUU_UTILS_H__
#define __YUU_UTILS_H__

#include <optional>
#include "sdl2/SDL.h"

namespace yuu {



	std::optional<SDL_Event> pollEvent();

}

#endif