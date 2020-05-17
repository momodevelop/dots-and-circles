#include <iostream>
#include "window.h"
#include "ryoji/utils/expected.h"
#include "ryoji/utils/fmt.h"
#include "canvas.h"

namespace  yuu::graphics {
	namespace window {
		using namespace ryoji;
		using namespace std;


		Window::Window(const Canvas& cv) : depCanvas(cv)
		{
			lastError.reserve(256);
		}

		Window::~Window() {
			assert(!this->window && "Please call free()");
		}

		Window::Expect<void> Window::init(const char* title, unsigned width, unsigned height)
		{
			if (!depCanvas.isInitialized()) {
				this->lastError = "[yuu][Window][init] Canvas not yet initialized!";
				return { Error() };
			}

			if (this->window) {
				this->lastError = "[yuu][Window][init] Window already created! Please free() first!";
				return { Error() };
			}

			this->window = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
			if (this->window == NULL) {
				this->lastError = fmt::staticFormat<256>("[yuu][Window][init] Cannot create window. Reason: %s", SDL_GetError());
				return { Error() };
			}
			return{};
		}

		Window::Expect<void> Window::free()
		{
			if (window) {
				SDL_DestroyWindow(this->window);
				this->window = nullptr;
				return {};
			}
			this->lastError = "[yuu][Window][free] Window was not created! Please init() first!";
			return { Error() };
		}

		const char* Window::getLastError() const
		{
			return this->lastError.c_str();
		}


	}

}

