#ifndef __YUU_UTILS_WINDOW_H__
#define __YUU_UTILS_WINDOW_H__


#include "sdl2/SDL.h"
#include "glew/glew.h"

namespace yuu::utils {

	// A simple window with an OpenGL context
	struct Window
	{
		SDL_Window* window{};
		SDL_GLContext context{};

		bool init(
			const char* title = "Test Project",
			int screenWidth = 1600,
			int screenHeight = 900,
			int screenX = SDL_WINDOWPOS_CENTERED,
			int screenY = SDL_WINDOWPOS_CENTERED,
			int openglMajorVersion = 3,
			int openglMinorVersion = 0
		);

		void free();
	};


}

#endif