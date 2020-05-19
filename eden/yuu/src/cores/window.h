#ifndef __YUU_CORE_WINDOW_H__
#define __YUU_CORE_WINDOW_H__

#include <functional>
#include "sdl2/SDL.h"

namespace yuu::cores {
	namespace windows {
		class Window {
			using UniqueWindow = std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)>;
			UniqueWindow window;

			Window(const Window&) = delete;
			Window& operator=(const Window&) = delete;
		public:
			Window(const char* title, int x, int y, int w, int h, Uint8 flags);
			~Window();
			Window(Window&&) = default;
			Window& operator=(Window&&) = default;



		};
	}

	using windows::Window;

}
#endif