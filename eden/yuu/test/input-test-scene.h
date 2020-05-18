#ifndef __TEST_SCENE_INPUT_H__
#define __TEST_SCENE_INPUT_H__

#include "scene.h"
#include "yuu/inputs/keyboard.h"

namespace yuu::cores::canvases { class Canvas; }
namespace scenes {
	class InputTestScene : public IScene {
		using Canvas = yuu::cores::canvases::Canvas;
		using Keyboard = yuu::input::keyboard::Keyboard;

		Keyboard kb{};
		Canvas& canvas;
		SceneDatabase& sceneDB;
		bool isDone = false;
	public:
		InputTestScene(SceneDatabase& sceneDB, Canvas& canvas);
		void init();
		IScene* update();
		void uninit();
	};
}


#endif