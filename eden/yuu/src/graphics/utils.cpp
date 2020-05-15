#include <iostream>
#include <iomanip>
#include "utils.h"
#include "sdl2/SDL.h"

namespace yuu::graphics {

	bool init() {
		if (SDL_Init(SDL_INIT_VIDEO) > 0) {
			return false;
		}
		SDL_GL_LoadLibrary(NULL);
		SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);

		//SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		//SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

		return true;
	};

	void free() {
		SDL_Quit();
	}

}





