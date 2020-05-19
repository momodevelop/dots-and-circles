#include <iostream>
#include <optional>

#include "glad/glad.h"
#include "ryoji/defer.h"
#include "yuu/inputs/keyboard.h"

#include "scene.h"
#include "input-test-scene.h"

#include "app.h"


int main(int argc, char* args[]){

	using namespace scenes;
	

	




	SceneDatabase sceneDB;
	InputTestScene inputTestScene(sceneDB);

	sceneDB.add("input", &inputTestScene);
	
	auto sceneMgr = SceneManager();
	sceneMgr.setNextScene(sceneDB.get("input"));
	sceneMgr.run();

	return 0;
}