#include <iostream>
#include <optional>

#include "ryoji/defer.h"
#include "yuu/utils.h"
#include "glad/glad.h"
#include "sdl2/SDL.h"

struct App {
	SDL_Window* window;
	SDL_GLContext context;
} app;

int main(int argc, char* args[]){
	
	if (SDL_Init(SDL_INIT_VIDEO) > 0) {
		std::cerr << SDL_GetError() << std::endl;
		return 1;
	}
	defer{
		std::cout << "Quitting" << std::endl;
		SDL_Quit();
	};

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
		return 1;
	}

	defer{
		std::cout << "Destroying Window " << std::endl;
		SDL_DestroyWindow(app.window);
	};

	app.context = SDL_GL_CreateContext(app.window);
	if (app.context == NULL) {
		std::cerr << SDL_GetError() << std::endl;
		return false;
	}
	defer{
		std::cout << "Destroying Context " << std::endl;
		SDL_GL_DeleteContext(app.context);
	};

	SDL_GL_SetSwapInterval(1);
	gladLoadGLLoader(SDL_GL_GetProcAddress);

	auto [w, h] = yuu::getWindowSize(app.window);
	glViewport(0, 0, w, h);
	glClearColor(0.0f, 0.5f, 1.0f, 0.0f);

	// now we poll
	bool isRunning = true;
	while (isRunning) {
		using namespace ryoji;
		auto e = yuu::pollEvent();
		if (maybe::isYes(e)) {
			if (maybe::get(e).type == SDL_QUIT) {
				break;
			}
		}
		
	}

	return 0;
}