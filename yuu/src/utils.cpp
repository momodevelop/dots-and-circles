#include <functional>
#include "utils.h"
#include "sdl2/SDL.h"

namespace yuu {
	using namespace ryoji;

	WindowSize getWindowSize(SDL_Window* window)
	{
		int w, h;
		SDL_GetWindowSize(window, &w, &h);
		return { w, h };
	}

	Maybe<SDL_Event> pollEvent()
	{
		SDL_Event ret;
		if (SDL_PollEvent(&ret)) {

			return maybe::make(ret);
		}
		return maybe::make<SDL_Event>();
	}
}