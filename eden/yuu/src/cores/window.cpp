#include "window.h"


namespace yuu::cores{

	namespace windows {
		Window::Window(const char* title, int x, int y, int w, int h, Uint8 flags) :
			window(SDL_CreateWindow(title, x, y, w, h, flags), SDL_DestroyWindow)
		{
		}
		Window::~Window()
		{
		}
	}
}