#include "input-test-scene.h"
#include "yuu/inputs/keyboard.h"
#include "yuu/utils.h"

#include <optional>
#include <iostream>

using namespace scenes;
using namespace std;
using namespace yuu::input::keyboard;
using namespace yuu::cores;


void checkKeyPressed(Keyboard& kb, Keycode key, const char* keyName) {
	using namespace yuu;
	using namespace std;
	if (isDown(kb, key))
		cout << keyName << " is down" << endl;
	if (isPoked(kb, key))
		cout << keyName << " is just pressed" << endl;
	if (isHeld(kb, key))
		cout << keyName << " is held" << endl;
	if (isReleased(kb, key))
		cout << keyName << " is just released" << endl;
}

InputTestScene::InputTestScene(SceneDatabase& sceneDB)
	: sceneDB(sceneDB)
{
}

void InputTestScene::init() {
	using namespace yuu::input;
	kb.registerKey(Keycode::W);
	kb.registerKey(Keycode::S);
	kb.registerKey(Keycode::A);
	kb.registerKey(Keycode::D);
}

IScene* InputTestScene::update() {
	using namespace yuu::input;
	kb.update();
	while (auto e = yuu::pollEvent()) {
		if (e->type == SDL_QUIT)
			return nullptr;
		else {
			kb.processEvent(e.value());
		}
	}
	checkKeyPressed(kb, Keycode::W, "W");
	checkKeyPressed(kb, Keycode::S, "A");
	checkKeyPressed(kb, Keycode::A, "S");
	checkKeyPressed(kb, Keycode::D, "D");




	return this;
}
void InputTestScene::uninit() {}



