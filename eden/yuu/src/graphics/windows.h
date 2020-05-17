#ifndef __YUU_GRAPHICS_WINDOW_H__
#define __YUU_GRAPHICS_WINDOW_H__

#include <string>
#include "sdl2/SDL.h"
#include "ryoji/utils/expected.h"

namespace yuu::graphics {
	namespace canvases { class Canvas; } // forward declaration
	namespace windows {
		// Is only in charge of wrapping an SDL_Window object.
		
		class Window
		{
			struct Error {};
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

			const char* getLastError() const;
			bool isInitialized() const;

			// Other gettors like window width, height etc...
		};


		
	}

	using windows::Window;
}


#endif