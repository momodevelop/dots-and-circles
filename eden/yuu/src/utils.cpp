#include "utils.h"


namespace yuu {
	std::optional<SDL_Event> pollEvent()
	{
		if (SDL_Event e; SDL_PollEvent(&e)) {
			return e;
		}
		return std::nullopt;
	}
}