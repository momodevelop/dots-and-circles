#ifndef __YUU_GRAPHICS_WINDOW_H__
#define __YUU_GRAPHICS_WINDOW_H__

#include <string>
#include "sdl2/SDL.h"
#include "ryoji/utils/expected.h"

namespace yuu::graphics {
	namespace canvas { class Canvas; } // forward declaration
	namespace window {
		// Is only in charge of wrapping an SDL_Window object.
		
		class Window
		{
			struct Error {};
			template<typename T> using Expect = ryoji::Expected<T, Error>;

			using Canvas = canvas::Canvas;
			const Canvas& depCanvas;
			SDL_Window* window{ nullptr };
			std::string lastError{};
		public:
			Window(const Canvas& cv);
			~Window();
			 
			Expect<void> init(const char* title, unsigned width, unsigned height);
			Expect<void> free();

			const char* getLastError() const;
		};


		
	}

	using window::Window;
}


#endif