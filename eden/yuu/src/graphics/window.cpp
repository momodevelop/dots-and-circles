#include <iostream>
#include <iomanip>
#include "window.h"
#include "glad.h"

namespace yuu::graphics::window {

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

		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);

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
		SDL_GL_MakeCurrent(w.window, w.context);


		GLfloat dim[2];
		glGetFloatv(GL_MAX_VIEWPORT_DIMS, dim);
		std::cout << "Maximum Viewport Dimensions: " << dim[0] << " x " << dim[1] << std::endl;



		
		return true;
	}

	void free(Window& w) {
		SDL_GL_DeleteContext(w.context);
		SDL_DestroyWindow(w.window);
		SDL_Quit();
	}
}




