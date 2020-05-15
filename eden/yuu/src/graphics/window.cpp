#include <iostream>
#include "window.h"


namespace  yuu::graphics {
	namespace window {
		Window::Window()
		{
		}

		Window::~Window() {
			SDL_DestroyWindow(window);
		}

		// TODO return expected?
		bool Window::init(const char* title, unsigned width, unsigned height)
		{
			if (this->window) {
				
			}

			this->window = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
			if (this->window == NULL) {
				std::cerr << "[yuu][createWindow] Cannot create window! SDL_Error: " << SDL_GetError() << std::endl;
				return false;
			}
		}

		bool Window::free()
		{
			if (window) {
				SDL_DestroyWindow(this->window);
				return true;
			}
			return false;
		}

	}

}

