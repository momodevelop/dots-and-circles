
#include "sdl2/SDL.h"
#include "canvas.h"
#include "glad/glad.h"
#include "ryoji/utils/fmt.h"

namespace yuu::cores {
	using namespace ryoji;

	namespace canvases {
		Canvas::Canvas() {
			assert(!isValid());
			assert(!this->constructed);
			this->constructed = true;
		}

		Canvas::~Canvas()
		{
			assert(!isValid());
			assert(this->constructed);
			this->constructed = false;
		}

		Canvas::Expect<void> Canvas::init()
		{
			assert(!isValid());

			if (SDL_Init(SDL_INIT_VIDEO) > 0) {
				return SDL_GetError();
			}

			SDL_GL_LoadLibrary(NULL);
			SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);

			//SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
			//SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

			gladLoadGLLoader(SDL_GL_GetProcAddress);

			this->initialized = true;

			return {};
		}
		void Canvas::free()
		{
			assert(isValid());
			SDL_Quit();
			this->initialized = false;
		}


		bool Canvas::isValid() const
		{
			return this->initialized;
		}

		std::optional<SDL_Event> Canvas::pollEvent()
		{
			assert(isValid());
			if (SDL_Event e; SDL_PollEvent(&e)) {
				return e;
			}
			return std::nullopt;
		}
	

	}

}

