#include <iostream>
#include <optional>
#include "yuu/utils/window.h"
#include "yuu/input/keyboard.h"
#include "ryoji/utils/defer.h"

using namespace yuu::input;
// Wrapper to change the awkward way SDL_PollEvent is being called.
static std::optional<SDL_Event> pollEvent() {
	SDL_Event e;
	bool success = SDL_PollEvent(&e);
	if (success) {
		return e;
	}
	return std::nullopt;
}


void checkKeyPressed(keyboard::Keyboard& kb, SDL_KeyCode key, const char* keyName) {
	using namespace yuu;
	using namespace std;
	if (keyboard::isDown(kb, key))
		cout << keyName << " is down" << endl;
	if (keyboard::isJustPressed(kb, key))
		cout << keyName << " is just pressed" << endl;
	if (keyboard::isHeld(kb, key))
		cout << keyName << " is held" << endl;
	if (keyboard::isJustReleased(kb, key))
		cout << keyName << " is just released" << endl;
}


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

	auto kb = keyboard::Keyboard{};
	keyboard::registerKey(kb, SDLK_w);
	keyboard::registerKey(kb, SDLK_s);
	keyboard::registerKey(kb, SDLK_a);
	keyboard::registerKey(kb, SDLK_d);

	bool quit = false; 
	SDL_Event e;
	int counter = 0;
	while (!quit) {
		keyboard::update(kb);
		while (auto e = pollEvent()){
			if (e->type == SDL_QUIT)
				quit = true;
			else {
				keyboard::processEvent(kb, e.value());
			}
		}

		checkKeyPressed(kb, SDLK_w, "W");
		checkKeyPressed(kb, SDLK_a, "A");
		checkKeyPressed(kb, SDLK_s, "S");
		checkKeyPressed(kb, SDLK_d, "D");


	}




	return 0;
}