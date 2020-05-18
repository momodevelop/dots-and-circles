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
			assert(cv.isInitialized());
			lastError.reserve(256);
		}

		Window::~Window() {
			assert(!isInitialized());
		}

		Window::Expect<void> Window::init(const char* title, unsigned width, unsigned height)
		{
			assert(!isInitialized() );
			this->window = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
			if (this->window == NULL) {
				this->lastError = fmt::staticFormat<256>("[yuu][Window][init] Cannot create window. Reason: %s", SDL_GetError());
				return { Error() };
			}
			return{};
		}

		void Window::free()
		{
			assert(isInitialized());
			SDL_DestroyWindow(this->window);
			this->window = nullptr;
		}

		const char* Window::getLastError() const
		{
			return this->lastError.c_str();
		}

		bool Window::isInitialized() const
		{
			return window != nullptr;
		}


	}

}

