#ifndef __YUU_GRAPHICS_WINDOW_H__
#define __YUU_GRAPHICS_WINDOW_H__

#include "sdl2/SDL.h"
#include "ryoji/utils/expected.h"

namespace yuu::graphics {

	namespace window {



		// A simple window with an OpenGL context
		class Window
		{
			using ErrorMessage = ryoji::Unexpected<std::string>;
			using MaybeError = ryoji::Expected<void, ErrorMessage>;
			SDL_Window* window{ nullptr };
		public:
			Window();
			~Window();
			 
			MaybeError init(const char* title, unsigned width, unsigned height);
			MaybeError free();
		};


		
	}

	using window::Window;
}


#endif