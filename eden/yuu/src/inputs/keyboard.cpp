#include <cassert>
#include "keyboard.h"

namespace yuu::input::keyboard {
	// Returns false if key is already registered.
	bool Keyboard::registerKey(Keycode key) {
		auto result = this->keys.try_emplace(key);
		return result.second;
	}

	bool Keyboard::unregisterKey(Keycode key) {
		auto result = this->keys.erase(key);
		return result > 0;
	}

	void Keyboard::update() {
		for (auto&& itr : this->keys) {
			itr.second.before = itr.second.now;
		}
	}

	void Keyboard::processEvent(const SDL_Event& e) {

		// Process the event
		if (e.type == SDL_KEYDOWN) {
			auto&& itr = this->keys.find(e.key.keysym.sym);
			if (itr != this->keys.end()) {
				itr->second.now = true;
			}

		}
		else if (e.type == SDL_KEYUP) {
			auto&& itr = this->keys.find(e.key.keysym.sym);
			if (itr != this->keys.end()) {
				itr->second.now = false;
			}
		}
	}

	inline const Keyboard::KeyStatus& Keyboard::getKeyStatus(Keycode key) const
	{
		auto&& itr = this->keys.find(key);
		assert(itr != this->keys.end());
		return itr->second;
	}


	// before: 0, now: 1
	bool isPoked(const Keyboard& keyboard, Keycode key) {
		auto keyStatus = keyboard.getKeyStatus(key);
		return !keyStatus.before && keyStatus.now;
	}

	// before: 1, now: 0
	bool isReleased(const Keyboard& keyboard, Keycode key) {
		auto keyStatus = keyboard.getKeyStatus(key);
		return keyStatus.before && !keyStatus.now;
	}


	// before: X, now: 1
	bool isDown(const Keyboard& keyboard, Keycode key) {
		return keyboard.getKeyStatus(key).now;
	}

	// before: 1, now: 1
	bool isHeld(const Keyboard& keyboard, Keycode key) {
		auto keyStatus = keyboard.getKeyStatus(key);
		return keyStatus.before && keyStatus.now;
	}
}