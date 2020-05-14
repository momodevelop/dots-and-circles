#ifndef __YUU_GRAPHICS_WINDOW_H__
#define __YUU_GRAPHICS_WINDOW_H__


#include "sdl2/SDL.h"


namespace yuu::graphics::window {

	// A simple window with an OpenGL context
	struct Window
	{
		SDL_Window* window{};
		SDL_GLContext context{};
	};

	bool init(
		Window& win,
		const char* title = "Test Project",
		int screenWidth = 1600,
		int screenHeight = 900,
		int screenX = SDL_WINDOWPOS_CENTERED,
		int screenY = SDL_WINDOWPOS_CENTERED,
		int openglMajorVersion = 4,
		int openglMinorVersion = 5
	);

	void free(Window& win);


}

#endif