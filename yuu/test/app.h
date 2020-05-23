#ifndef __APP_H__
#define __APP_H__

#include "sdl2/SDL.h"

#include "states.h"
#include "states/input.h"

namespace app {
	struct App {
		State state;
		SDL_Window* window;
		SDL_GLContext context;
		states::input::InputState inputState;
	};

	bool init(App& app);
	void run(App& app);
	void free(App& app);

}

using app::App;





#endif