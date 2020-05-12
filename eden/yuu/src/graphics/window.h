#ifndef __YUU_GRAPHICS_WINDOW_H__
#define __YUU_GRAPHICS_WINDOW_H__


#include "sdl2/SDL.h"
#include "glew/glew.h"

namespace yuu::graphics {

	// A simple window with an OpenGL context
	struct Window
	{
		SDL_Window* window{};
	};

	bool init(
		Window& win,
		const char* title = "Test Project",
		int screenWidth = 1600,
		int screenHeight = 900,
		int screenX = SDL_WINDOWPOS_CENTERED,
		int screenY = SDL_WINDOWPOS_CENTERED,
		int openglMajorVersion = 3,
		int openglMinorVersion = 0
	);

	void free(Window& win);


}

#endif