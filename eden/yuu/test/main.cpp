#include <iostream>
#include <optional>
#include "yuu/graphics/utils.h"
#include "yuu/graphics/window.h"
#include "yuu/input/keyboard.h"
#include "ryoji/utils/defer.h"

#include "scene.h"
#include "input-test-scene.h"

using namespace yuu::input;



int main(int argc, char* args[]){
	using namespace yuu::graphics;
	using namespace yuu;
	using namespace scenes;

	if (!graphics::init()) {
		std::cerr << "Cannot initialize graphics" << std::endl;
		return 1;
	}
	defer{ graphics::free(); };

	auto win = graphics::Window();
	
	if (auto ok = win.init("Yuu Test", 1600, 900); !ok ) {
		std::cerr << ok.getError().getValue() << std::endl;
		return 1;
	}
	defer{ win.free(); };

	
	auto sceneMgr = SceneManager();
	sceneMgr.setNextScene(InputTestScene::I());
	sceneMgr.run();

	return 0;
}