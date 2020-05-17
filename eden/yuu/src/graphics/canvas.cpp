
#include <sdl2/SDL.h>
#include "canvas.h"
#include "ryoji/utils/fmt.h"

namespace yuu::graphics {
	using namespace ryoji;

	namespace canvas {
		Canvas::Canvas() {
			lastError.reserve(256);
		}

		Canvas::~Canvas()
		{
			assert(this->initialized && "Please call free()");
		}

		Canvas::Expect<void> Canvas::init()
		{
			if (this->initialized) {
				lastError = fmt::staticFormat<256>("[yuu][Canvas][init] Already Initialized! Please call free()");
				return Error();
			}
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


			this->initialized = true;

			return {};
		}
		Canvas::Expect<void> Canvas::free()
		{
			if (!this->initialized) {
				lastError = fmt::staticFormat<256>("[yuu][Canvas][free] Did not initialize before free()");
				return Error();
			}
			SDL_Quit();
			this->initialized = false;
			return {};
		}

		const char* Canvas::getLastError() const
		{
			return lastError.c_str();
		}

		bool Canvas::isInitialized() const
		{
			return initialized;
		}

	}

}

