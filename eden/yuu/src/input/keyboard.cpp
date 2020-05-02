#include <cassert>
#include "ryoji/maths/bitwise.h"
#include "keyboard.h"

namespace yuu::input::keyboard {
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
			auto&& itr = keyboard.keys.find(e.key.keysym.sym);
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