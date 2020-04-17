#ifndef __RYOJI_DISPLAY_WINDOW_H__
#define __RYOJI_DISPLAY_WINDOW_H__

#include <iostream>
#include "SDL.h"
#include "glew.h"

namespace ryoji::display {


	// A simple window with an OpenGL context
	struct StdWin
	{
		SDL_Window* window{};
		SDL_GLContext* context{};

		bool init(
			const char* title,
			int screenWidth,
			int screenHeight,
			int screenX = SDL_WINDOWPOS_UNDEFINED,
			int screenY = SDL_WINDOWPOS_UNDEFINED,
			int oglMajorVersion = 3,
			int oglMinorVersion = 0
		) {


			//Initialize SDL
			if (SDL_Init(SDL_INIT_VIDEO) < 0)
			{
				std::cout << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
				return {};
			}

			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, oglMajorVersion);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, oglMinorVersion);

			this->window = SDL_CreateWindow(title, screenX, screenY, screenWidth, screenHeight, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
			if (window == NULL)
			{
				std::cout << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
				return false;
			}

			auto context = SDL_GL_CreateContext(window);
			if (context == NULL)
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
	};


}

#endif