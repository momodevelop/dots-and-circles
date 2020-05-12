#include <iostream>
#include <optional>
#include "yuu/graphics/window.h"
#include "yuu/input/keyboard.h"
#include "ryoji/utils/defer.h"

#include "scene.h"
#include "input-test-scene.h"

using namespace yuu::input;



int main(int argc, char* args[]){
	using namespace yuu::graphics;
	using namespace yuu;
	using namespace std;
	using namespace scenes;
	auto win = graphics::window::Window();
	if (!window::init(win, "Yuu Test")) {
		cout << "Cannot initialize window" << endl;
		return 1;
	}
	defer{ free(win); };

	
	auto sceneMgr = SceneManager();
	sceneMgr.setNextScene(InputTestScene::I());
	sceneMgr.run();

	return 0;
}