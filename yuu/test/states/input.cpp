#include <iostream>

#include "yuu/utils.h"
#include "input.h"
#include "../states.h"


namespace app::states::input {
	void init(InputState& state)
	{
	}

	State run(InputState& state) {
		init(state);

		bool isRunning = true;
		while (isRunning) {
			using namespace ryoji::maybe;
			using namespace yuu;
			auto e = yuu::pollEvent();
			if (isYes(e)) {
				if (get(e).type == SDL_QUIT) {
					break;
				}
			}
		}



		free(state);
		return State::NONE;
	}
	void free(InputState& state)
	{
	}
}