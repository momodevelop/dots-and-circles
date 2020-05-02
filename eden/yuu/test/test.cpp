#include <iostream>
#include <optional>
#include "yuu/utils/window.h"
#include "ryoji/utils/defer.h"


// Wrapper to change the awkward way SDL_PollEvent is being called.
static std::optional<SDL_Event> pollEvent() {
	SDL_Event e;
	bool success = SDL_PollEvent(&e);
	if (success) {
		return e;
	}
	return std::nullopt;
}

// Typedefs for SDL2


#include <unordered_map>
#include <bitset>
#include <cassert>
#include "ryoji/maths/bitwise.h"
// A keyboard component
namespace yuu::keyboard {
	// We want to represent 4 states of button pressing:
	// 1. Not pressed at all
	// 2. Just been pressed.
	// 3. Pressed for some time (held)
	// 4. Just released
	//
	// Because we have 4 states, we can (in theory) represent it using 2 boolean variables
	// +--------------+---------+------------------+
	// | WAS_PRESSED  | PRESSED | State            |
	// +--------------+---------+------------------+
	// |  0           |  0      | Not pressed      |
	// +--------------+---------+------------------+
	// |  0           |  1      | Just pressed     |
	// +--------------+---------+------------------+
	// |  1           |  1      | Is held	       |
	// +--------------+---------+------------------+
	// |  1           |  0      | Is released      |
	// +--------------+---------+------------------+
	//

	struct KeyStatus {
		bool pressed : 1;
		bool wasPressed : 1;
	};

	struct Keyboard {
		std::unordered_map<SDL_Keycode, KeyStatus> keys{};
	};


	// Returns false if key is already registered.
	bool registerKey(Keyboard& keyboard, SDL_Keycode key) {
		auto result = keyboard.keys.try_emplace(key);
		return result.second;
	}

	bool unregisterKey(Keyboard& keyboard, SDL_Keycode key) {
		auto result = keyboard.keys.erase(key);
		return result > 0;
	}

	void update(Keyboard& keyboard) {
		for (auto&& itr : keyboard.keys) {
			itr.second.wasPressed = itr.second.pressed;
		}
	}

	void processEvent(Keyboard& keyboard, const SDL_Event& e) {
		using namespace ryoji::maths;


		// Process the event
		if (e.type == SDL_KEYDOWN) {
			auto&& itr = keyboard.keys.find(e.key.keysym.sym) ;
			if (itr != keyboard.keys.end()) {
				itr->second.pressed = true;
			}
			
		}
		else if (e.type == SDL_KEYUP) {
			auto&& itr = keyboard.keys.find(e.key.keysym.sym);
			if (itr != keyboard.keys.end()) {
				itr->second.pressed = false;
			}
		}
	}


	// WAS_PRESSED: 0, PRESSED: 1
	bool isJustPressed(const Keyboard& keyboard, SDL_KeyCode key) {
		using namespace ryoji::maths;
		auto&& itr = keyboard.keys.find(key);
		assert(itr != keyboard.keys.end());
		return !itr->second.wasPressed && itr->second.pressed;
	}

	// WAS_PRESSED: 1, PRESSED: 0
	bool isJustReleased(const Keyboard& keyboard, SDL_KeyCode key) {
		using namespace ryoji::maths;
		auto&& itr = keyboard.keys.find(key);
		assert(itr != keyboard.keys.end());
		return itr->second.wasPressed && !itr->second.pressed;

	}


	// WAS_PRESSED: X, PRESSED: 1
	bool isDown(const Keyboard& keyboard, SDL_KeyCode key) {
		using namespace ryoji::maths;
		auto&& itr = keyboard.keys.find(key);
		assert(itr != keyboard.keys.end());
		return itr->second.pressed;
	}

	// WAS_PRESSED: 1, PRESSED: 1
	bool isHeld(const Keyboard& keyboard, SDL_KeyCode key) {
		using namespace ryoji::maths;
		auto&& itr = keyboard.keys.find(key);
		assert(itr != keyboard.keys.end());
		return itr->second.wasPressed && itr->second.pressed;
	}
}

void checkKeyPressed(yuu::keyboard::Keyboard& kb, SDL_KeyCode key, const char * keyName) {
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
		cout << counter++ << endl;
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