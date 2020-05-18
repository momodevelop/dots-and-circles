#include <iostream>
#include <optional>
#include "yuu/graphics/canvases.h"
#include "yuu/graphics/windows.h"
#include "yuu/inputs/keyboard.h"
#include "ryoji/utils/defer.h"

#include "scene.h"
#include "input-test-scene.h"

int main(int argc, char* args[]){
	using namespace yuu;
	using namespace scenes;

	auto canvas = graphics::Canvas();
	if (auto ok = canvas.init(); !ok) {
		std::cerr << canvas.getLastError() << std::endl;
		return 1;
	}
	defer{ canvas.free(); };

	auto win = graphics::Window(canvas);
	if (auto ok = win.init("Yuu Test", 1600, 900); !ok ) {
		std::cerr << win.getLastError() << std::endl;
		return 1;
	}
	defer{ win.free(); };

	SceneDatabase sceneDB;

	InputTestScene inputTestScene(sceneDB, canvas);

	sceneDB.add("input", &inputTestScene);
	
	auto sceneMgr = SceneManager();
	sceneMgr.setNextScene(sceneDB.get("input"));
	sceneMgr.run();

	return 0;
}