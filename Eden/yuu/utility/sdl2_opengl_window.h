#ifndef __YUU_SDL2_OPENGL_WINDOW_H__
#define __YUU_SDL2_OPENGL_WINDOW_H__

#include <iostream>
#include "../../ryoji/utility/defer.h"
#include "../../deps/sdl2/include/SDL.h"
#include "../../deps/glew/include/glew.h"

namespace yuu::utility {

	// A simple window with an OpenGL context
	struct SDL2OpenGLWindow
	{
		SDL_Window* window{};
		SDL_GLContext context{};

		bool init(
			const char* title = "Test Project",
			int screenWidth = 1600,
			int screenHeight = 900,
			int screenX = SDL_WINDOWPOS_CENTERED,
			int screenY = SDL_WINDOWPOS_CENTERED,
			int openglMajorVersion = 3,
			int openglMinorVersion = 0

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

		void free() {
			SDL_GL_DeleteContext(this->context);
			SDL_DestroyWindow(this->window);
			SDL_Quit();
		}
	};


}

#endif