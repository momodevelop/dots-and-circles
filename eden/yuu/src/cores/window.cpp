#include <iostream>

#include "ryoji/utils/expected.h"
#include "ryoji/utils/fmt.h"

#include "canvas.h"
#include "window.h"

namespace  yuu::cores {
	namespace windows {
		using namespace ryoji;
		using namespace std;


		Window::Window(const Canvas& cv) : canvas(cv)
		{
			assert(cv.isValid());
			lastError.reserve(256);
		}

		Window::~Window() {
			assert(!isValid());
		}

		Window::Expect<void> Window::init(const char* title, unsigned width, unsigned height)
		{
			assert(!isValid() );
			this->window = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
			if (this->window == NULL) {
				return SDL_GetError() ;
			}
			return{};
		}

		void Window::free()
		{
			assert(isValid());
			SDL_DestroyWindow(this->window);
			this->window = nullptr;
		}

		bool Window::isValid() const
		{
			return window != nullptr;
		}


	}

}

