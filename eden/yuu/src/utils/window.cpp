#include <iostream>
#include "window.h"

namespace yuu::utils::window {
	bool init(
		Window& w,
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

		w.window = SDL_CreateWindow(title, screenX, screenY, screenWidth, screenHeight, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
		if (w.window == NULL)
		{
			std::cout << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
			return false;
		}

		w.context = SDL_GL_CreateContext(w.window);
		if (w.context == NULL)
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

	void free(Window& w) {
		SDL_GL_DeleteContext(w.context);
		SDL_DestroyWindow(w.window);
		SDL_Quit();
	}
}




