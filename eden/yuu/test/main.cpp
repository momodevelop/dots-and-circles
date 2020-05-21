#include <iostream>
#include <optional>

#include "ryoji/defer.h"
#include "glad/glad.h"
#include "sdl2/SDL.h"
#include "app.h"


int main(int argc, char* args[]){
	if (SDL_Init(SDL_INIT_VIDEO) > 0) {
		std::cerr << SDL_GetError() << std::endl;
		exit(1);
	}
	defer{ SDL_Quit(); };

	SDL_GL_LoadLibrary(NULL);
	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);

	auto window = SDL_CreateWindow("Test",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		1600, 900, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);




	App app;
	app.run();


	//maincontext = SDL_GL_CreateContext(window);
	//if (maincontext == NULL)
	//	sdl_die("Failed to create OpenGL context");

	SDL_GL_SetSwapInterval(1);


	gladLoadGLLoader(SDL_GL_GetProcAddress);
	//glDisable(GL_DEPTH_TEST);
	//glDisable(GL_CULL_FACE);

	int w, h;
	SDL_GetWindowSize(window, &w, &h);
	glViewport(0, 0, w, h);
	glClearColor(0.0f, 0.5f, 1.0f, 0.0f);


	return 0;
}