#include <iostream>
#include "window.h"
#include "ryoji/utils/expected.h"
#include "ryoji/utils/fmt.h"

namespace  yuu::graphics {
	namespace window {
		using namespace ryoji;
		using namespace std;

		Window::Window()
		{
		}

		Window::~Window() {
			free();
		}

		Window::MaybeError Window::init(const char* title, unsigned width, unsigned height)
		{
			if (this->window) {
				return { fmt::format<256>("[yuu][Window][init] Window already created! Please free() first!") };
			}

			this->window = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
			if (this->window == NULL) {
				return { fmt::format<256>("[yuu][Window][init] Cannot create window. Reason: %s", SDL_GetError()) };
			}
			return{};
		}

		Window::MaybeError Window::free()
		{
			if (window) {
				SDL_DestroyWindow(this->window);
				return {};
			}
			return { string("[yuu][Window][free] Window was not created! Please init() first!") };
		}


	}

}

