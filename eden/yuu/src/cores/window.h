#ifndef __YUU_CORE_WINDOW_H__
#define __YUU_CORE_WINDOW_H__

#include <string>
#include "sdl2/SDL.h"
#include "ryoji/utils/expected.h"

namespace yuu::cores {
	namespace canvases { class Canvas; } // forward declaration
	namespace windows {
		// Is only in charge of wrapping an SDL_Window object.
		
		class Window
		{
			using Error = const char*;
			template<typename T> using Expect = ryoji::Expected<T, Error>;
			using Canvas = canvases::Canvas;

			// dependencies
			const Canvas& canvas;

			SDL_Window* window{ nullptr };
			std::string lastError{};
		public:
			Window(const Canvas& cv);
			~Window();
			 
			Expect<void> init(const char* title, unsigned width, unsigned height);
			void free();

			bool isValid() const;

		};
	}

	using windows::Window;


	namespace renderers {
		class Renderer {
			using Window = windows::Window;

			// dependencies
			const Window& window;
		public:
			Renderer(const Window& window);

		};
	}
}


#endif