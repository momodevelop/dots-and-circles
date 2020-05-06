#include <iostream>
#include <optional>
#include "yuu/utils/window.h"
#include "yuu/input/keyboard.h"
#include "ryoji/utils/defer.h"

#include "scene.h"
#include "input-test-scene.h"

using namespace yuu::input;



int main(int argc, char* args[]){
	using namespace yuu::utils;
	using namespace yuu;
	using namespace std;
	using namespace scenes;
	auto win = window::Window();
	if (!window::init(win, "Yuu Test")) {
		cout << "Cannot initialize window" << endl;
		return 1;
	}
	defer{ window::free(win); };

	
	SceneManager<int> sceneMgr;
	sceneMgr.registerScene<InputTestScene>(0);

	
	
	if (!sceneMgr.isDone())
		sceneMgr.update();

	




	return 0;
}