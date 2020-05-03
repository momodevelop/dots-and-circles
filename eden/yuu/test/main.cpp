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

	auto win = window::Window();
	if (!window::init(win, "Yuu Test")) {
		cout << "Cannot initialize window" << endl;
		return 1;
	}
	defer{ window::free(win); };

	auto scene = new scenes::InputTestScene();
	defer{ delete scene; };

	scene->init();
	while (!scene->done()) {
		scene->update();
	}
	scene->cleanup();




	return 0;
}