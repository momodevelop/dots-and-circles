#include <iostream>
#include "ryoji/defer.h"
#include "ryoji/allocators/utils.h"
#include "sdl2/SDL.h"
#include "glad/glad.h"

#include "yuu/utils.h"
#include "app.h"



App::App()
{
	using namespace std;
	using namespace ryoji;
	using namespace allocators;

	if (SDL_Init(SDL_INIT_VIDEO) > 0) {
		cerr << SDL_GetError() << endl;
		exit(1);
	}
	defer{ SDL_Quit(); };

	SDL_GL_LoadLibrary(NULL);
	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);

	//SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	//SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

	gladLoadGLLoader(SDL_GL_GetProcAddress);


	this->window = shared_ptr<Window>(
		construct<Window>(allocator,
			"Test",
			SDL_WINDOWPOS_UNDEFINED,
			SDL_WINDOWPOS_UNDEFINED,
			1600, 900, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN)
		,
		[=](Window* window) {
			destruct<Window>(allocator, window);
		});

	//this->window = shared_ptr<Window>(construct<Window>(allocator), [=](Window* window) {this->window)
		
		

/*	this->window = yuu::createSharedWindow(
		"Test",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		1600, 900, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

	if (!window) {
		std::cerr << SDL_GetError() << std::endl;
		exit(1);
	}

	SDL_GL_SetSwapInterval(1);

	int w, h;
	SDL_GetWindowSize(window.get(), &w, &h);
	glViewport(0, 0, w, h);
	glClearColor(0.0f, 0.5f, 1.0f, 0.0f);


	auto context = SDL_GL_CreateContext(window);
	if (context == NULL) {
		std::cerr << SDL_GetError() << std::endl;
		return 1;
	}
	*/

}

App::~App()
{
}
