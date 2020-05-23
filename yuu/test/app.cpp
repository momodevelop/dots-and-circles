#include <iostream>


#include "yuu/utils.h"
#include "glad/glad.h"

#include "app.h"
#include "states/input.h"

namespace app {
	bool init(App& app) {
		if (SDL_Init(SDL_INIT_VIDEO) > 0) {
			std::cerr << SDL_GetError() << std::endl;
			return false;
		}

		SDL_GL_LoadLibrary(NULL);
		SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);


		app.window = SDL_CreateWindow("Test",
			SDL_WINDOWPOS_UNDEFINED,
			SDL_WINDOWPOS_UNDEFINED,
			1600, 900, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
		if (!app.window) {
			std::cerr << SDL_GetError() << std::endl;
			return false;
		}

		app.context = SDL_GL_CreateContext(app.window);
		if (app.context == NULL) {
			std::cerr << SDL_GetError() << std::endl;
			return false;
		}
		SDL_GL_SetSwapInterval(1);
		gladLoadGLLoader(SDL_GL_GetProcAddress);

		auto [w, h] = yuu::getWindowSize(app.window);
		glViewport(0, 0, w, h);
		glClearColor(0.0f, 0.5f, 1.0f, 0.0f);

		app.state = State::KEYBOARD;

		return true;
	}


	void free(App& app) {
		SDL_GL_DeleteContext(app.context);
		SDL_DestroyWindow(app.window);
		SDL_Quit();
	}

	void run(App& app) {
		using namespace states;
		while (app.state != State::NONE) {
			switch (app.state) {
				case State::KEYBOARD:
				{
					app.state = input::run(app.inputState);
				} break;
				default:
				{
				}
			}
		}

	}
}