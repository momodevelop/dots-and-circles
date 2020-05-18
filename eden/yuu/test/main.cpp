#include <iostream>
#include <optional>

#include "glad/glad.h"
#include "ryoji/defer.h"
#include "yuu/inputs/keyboard.h"

#include "scene.h"
#include "input-test-scene.h"




int main(int argc, char* args[]){

	using namespace scenes;
	if (SDL_Init(SDL_INIT_VIDEO) > 0) {
		std::cerr << SDL_GetError() << std::endl;
		return 1;
	}
	defer{ SDL_Quit(); };

	SDL_GL_LoadLibrary(NULL);
	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);

	//SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	//SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

	gladLoadGLLoader(SDL_GL_GetProcAddress);


	auto window = SDL_CreateWindow("Test", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1600, 900, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
	if (window == NULL) {
		std::cerr << SDL_GetError() << std::endl;
		return 1;
	}
	defer{ SDL_DestroyWindow(window); };


	SceneDatabase sceneDB;
	InputTestScene inputTestScene(sceneDB);

	sceneDB.add("input", &inputTestScene);
	
	auto sceneMgr = SceneManager();
	sceneMgr.setNextScene(sceneDB.get("input"));
	sceneMgr.run();

	return 0;
}