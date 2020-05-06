#ifndef __TEST_SCENE_INPUT_H__
#define __TEST_SCENE_INPUT_H__

#include "scene.h"
#include "yuu/input/keyboard.h"

namespace scenes {
	class InputTestScene {
		yuu::input::keyboard::Keyboard kb{};
		bool isDone = false;
	public:
		void init();
		void update();
		void uninit();
		bool done();
	};
}


#endif