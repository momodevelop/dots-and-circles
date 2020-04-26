#include <iostream>
#include "window.h"

namespace yuu::utils {
	bool Window::init(
		const char* title,
		int screenWidth,
		int screenHeight,
		int screenX,
		int screenY,
		int openglMajorVersion,
		int openglMinorVersion

	) {

		//Initialize SDL
		if (SDL_Init(SDL_INIT_VIDEO) < 0)
		{
			std::cout << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
			return false;
		}

		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, openglMajorVersion);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, openglMinorVersion);

		this->window = SDL_CreateWindow(title, screenX, screenY, screenWidth, screenHeight, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
		if (this->window == NULL)
		{
			std::cout << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
			return false;
		}

		this->context = SDL_GL_CreateContext(window);
		if (this->context == NULL)
		{
			std::cout << "OpenGL context could not be created! SDL_Error: " << SDL_GetError() << std::endl;
			return false;
		}

		// Standard openGL initialization
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		auto error = glGetError();
		if (error != GL_NO_ERROR)
		{
			printf("Error initializing OpenGL! %s", glewGetErrorString(error));
			return false;
		}
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glClearColor(0.f, 0.f, 0.f, 0.f);

		return true;
	}

	void Window::free() {
		SDL_GL_DeleteContext(this->context);
		SDL_DestroyWindow(this->window);
		SDL_Quit();
	}
}




