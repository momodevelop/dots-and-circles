#include <functional>
#include "utils.h"
#include "sdl2/SDL.h"

namespace yuu {
	std::pair<int, int> getWindowSize(SDL_Window* window)
	{
		int w, h;
		SDL_GetWindowSize(window, &w, &h);
		return { w, h };
	}
	SDL_Event* pollEvent()
	{
		SDL_Event* ret = nullptr;
		SDL_Event ret2;
		if (SDL_PollEvent(&ret2)) {

			return ret;
		}
		return nullptr;
	}
}