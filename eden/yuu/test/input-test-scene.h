#ifndef __TEST_SCENE_INPUT_H__
#define __TEST_SCENE_INPUT_H__

#include "scene.h"
#include "yuu/input/keyboard.h"

namespace scenes {
	class InputTestScene : public IScene {
		yuu::input::keyboard::Keyboard kb{};
		bool isDone = false;
	public:
		void init();
		IScene* update();
		void uninit();

		static InputTestScene* I() {
			static InputTestScene instance;
			return &instance;
		}
	};
}


#endif