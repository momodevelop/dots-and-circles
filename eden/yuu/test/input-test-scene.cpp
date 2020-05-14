#include "input-test-scene.h"
#include "yuu/input/keyboard.h"

#include <optional>
#include <iostream>

using namespace scenes;
using namespace std;
using namespace yuu::input::keyboard;



// Wrapper to change the awkward way SDL_PollEvent is being called.
static std::optional<SDL_Event> pollEvent() {
	SDL_Event e;
	bool success = SDL_PollEvent(&e);
	if (success) {
		return e;
	}
	return std::nullopt;
}

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
	while (auto e = pollEvent()) {
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



