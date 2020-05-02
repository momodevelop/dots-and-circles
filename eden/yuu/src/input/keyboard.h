#ifndef __YUU_INPUT_KEYBOARD_H__
#define __YUU_INPUT_KEYBOARD_H__


#include <unordered_map>
#include <bitset>

#include "sdl2/SDL.h"

// A keyboard component
namespace yuu::input::keyboard {
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
	// Order of function call should be:
	// registerKeys() -> game loop
	//						|
	//						---> processEvent() -> key checking functions -> update()


	struct KeyStatus {
		bool pressed : 1;
		bool wasPressed : 1;
	};

	struct Keyboard {
		std::unordered_map<SDL_Keycode, KeyStatus> keys{};
	};

	bool registerKey(Keyboard& keyboard, SDL_Keycode key);
	bool unregisterKey(Keyboard& keyboard, SDL_Keycode key);
	void update(Keyboard& keyboard);
	void processEvent(Keyboard& keyboard, const SDL_Event& e);
	
	// checks
	bool isJustPressed(const Keyboard& keyboard, SDL_KeyCode key);
	bool isJustReleased(const Keyboard& keyboard, SDL_KeyCode key);
	bool isDown(const Keyboard& keyboard, SDL_KeyCode key);
	bool isHeld(const Keyboard& keyboard, SDL_KeyCode key);
}



#endif