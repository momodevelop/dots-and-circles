#ifndef __YUU_GRAPHICS_WINDOW_H__
#define __YUU_GRAPHICS_WINDOW_H__

#include "sdl2/SDL.h"

namespace yuu::graphics {

	namespace window {
		// A simple window with an OpenGL context
		class Window
		{
			SDL_Window* window{ nullptr };
		public:
			Window();
			~Window();

			bool init(const char* title, unsigned width, unsigned height);
			bool free();
		};
	}

	using window::Window;
}


#endif