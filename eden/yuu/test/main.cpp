#include <iostream>
#include <optional>
#include "yuu/graphics/canvas.h"
#include "yuu/graphics/window.h"
#include "yuu/input/keyboard.h"
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



	
	auto sceneMgr = SceneManager();
	sceneMgr.setNextScene(InputTestScene::I());
	sceneMgr.run();

	return 0;
}