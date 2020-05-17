#ifndef __TEST_SCENE_INPUT_H__
#define __TEST_SCENE_INPUT_H__

#include "scene.h"
#include "yuu/input/keyboard.h"

namespace yuu::graphics::canvases { class Canvas; }
namespace scenes {
	class InputTestScene : public IScene {
		yuu::input::keyboard::Keyboard kb{};
		yuu::graphics::canvases::Canvas& canvas;
		SceneDatabase& sceneDB;
		bool isDone = false;
	public:
		InputTestScene(SceneDatabase& sceneDB, yuu::graphics::canvases::Canvas& canvas);
		void init();
		IScene* update();
		void uninit();
	};
}


#endif