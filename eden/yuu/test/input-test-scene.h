#ifndef __TEST_SCENE_INPUT_H__
#define __TEST_SCENE_INPUT_H__

#include "scene.h"
#include "yuu/inputs/keyboard.h"

namespace yuu::cores::canvases { class Canvas; }
namespace scenes {
	class InputTestScene : public IScene {
		using Keyboard = yuu::input::keyboard::Keyboard;

		Keyboard kb{};
		SceneDatabase& sceneDB;
		bool isDone = false;
	public:
		InputTestScene(SceneDatabase& sceneDB);
		void init();
		IScene* update();
		void uninit();
	};
}


#endif