
#include <sdl2/SDL.h>
#include "canvas.h"
#include "ryoji/utils/fmt.h"

namespace yuu::cores {
	using namespace ryoji;

	namespace canvases {
		Canvas::Canvas() {
			lastError.reserve(256);
		}

		Canvas::~Canvas()
		{
			assert(isInitialized());
		}

		Canvas::Expect<void> Canvas::init()
		{
			assert(!isInitialized());

			if (SDL_Init(SDL_INIT_VIDEO) > 0) {
				lastError = fmt::staticFormat<256>("[yuu][Canvas][init] SDL cannot initialize: %s", SDL_GetError());
				return Error();
			}

			SDL_GL_LoadLibrary(NULL);
			SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);

			//SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
			//SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

			#ifdef _DEBUG
				this->initialized = true;
			#endif

			return {};
		}
		void Canvas::free()
		{
			assert(isInitialized());
			SDL_Quit();
#ifdef _DEBUG
			this->initialized = false;
#endif
		}

		const char* Canvas::getLastError() const
		{
			return lastError.c_str();
		}
#ifdef _DEBUG
		bool Canvas::isInitialized() const
		{
			return initialized;
		}
#endif
		std::optional<SDL_Event> Canvas::pollEvent()
		{
			assert(isInitialized());
			if (SDL_Event e; SDL_PollEvent(&e)) {
				return e;
			}
			return std::nullopt;
		}
	

	}

}

