#include "input-test-scene.h"
#include "yuu/inputs/keyboard.h"
#include "yuu/cores/canvas.h"

#include <optional>
#include <iostream>

using namespace scenes;
using namespace std;
using namespace yuu::input::keyboard;
using namespace yuu::cores;


void checkKeyPressed(Keyboard& kb, SDL_KeyCode key, const char* keyName) {
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

InputTestScene::InputTestScene(SceneDatabase& sceneDB, Canvas& canvas)
	: sceneDB(sceneDB), canvas(canvas)
{
}

void InputTestScene::init() {
	using namespace yuu::input;
	kb.registerKey(SDLK_w);
	kb.registerKey(SDLK_s);
	kb.registerKey(SDLK_a);
	kb.registerKey(SDLK_d);
}

IScene* InputTestScene::update() {
	using namespace yuu::input;
	kb.update();
	while (auto e = canvas.pollEvent()) {
		if (e->type == SDL_QUIT)
			return nullptr;
		else {
			kb.processEvent(e.value());
		}
	}
	checkKeyPressed(kb, SDLK_w, "W");
	checkKeyPressed(kb, SDLK_a, "A");
	checkKeyPressed(kb, SDLK_s, "S");
	checkKeyPressed(kb, SDLK_d, "D");




	return this;
}
void InputTestScene::uninit() {}



