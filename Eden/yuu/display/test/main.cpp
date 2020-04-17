#include <iostream>
#include "../include/ryoji-display.h"
using namespace std;



int SDL_main(int argc, char* argv[]) {
	//Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		cout << "SDL could not initialize! SDL_Error: " << SDL_GetError() << endl;
		return 1;
	}

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

	auto window = SDL_CreateWindow("Ryoji Display Sandbox", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
	if (window == NULL)
	{
		cout << "Window could not be created! SDL_Error: " << SDL_GetError() << endl;;
		return 1;
	}

	auto context = SDL_GL_CreateContext(window);
	if (context == NULL)
	{
		cout << "OpenGL context could not be created! SDL_Error: " << SDL_GetError() << endl;;
		return 1;
	}

	// OpenGL related



	SDL_UpdateWindowSurface(window);
	SDL_Delay(2000);
	

	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}