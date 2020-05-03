#ifndef __TEST_SCENE_INPUT_H__
#define __TEST_SCENE_INPUT_H__

#include "scene.h"
#include "yuu/input/keyboard.h"

namespace scenes {
	class InputTestScene : public IScene {
		yuu::input::keyboard::Keyboard kb{};
		bool isDone = false;
	public:
		void init() override;
		void update() override;
		void cleanup() override;
		bool done() override;
	};
}


#endif