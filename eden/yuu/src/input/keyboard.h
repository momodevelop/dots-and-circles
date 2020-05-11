#ifndef __YUU_INPUT_KEYBOARD_H__
#define __YUU_INPUT_KEYBOARD_H__


#include <unordered_map>
#include <bitset>

#include "sdl2/SDL.h"

// A keyboard component
namespace yuu::input {
	// We want to represent 4 states of button pressing:
	// 1. Not pressed at all
	// 2. Just been pressed.
	// 3. Pressed for some time (held)
	// 4. Just released
	//
	// Because we have 4 states, we can (in theory) represent it using 2 boolean variables.
	// 'now' represents if the button is being pressed at the current frame.
	// 'before' represents if the button is being pressed at the previous frame.
	// +--------+------+------------------+
	// | before | now  | meaning          |
	// +--------+------+------------------+
	// |  0     |  0   | Not pressed      |
	// +--------+------+------------------+
	// |  0     |  1   | Just pressed     |
	// +--------+------+------------------+
	// |  1     |  1   | Is held	      |
	// +--------+------+------------------+
	// |  1     |  0   | Is released      |
	// +--------+------+------------------+
	//
	// Order of function call should be:
	// registerKeys() -> game loop
	//						|
	//						---> processEvent() -> key checking functions -> update()

	namespace keyboard {
		class Keyboard {
			struct KeyStatus {
				bool now : 1;
				bool before : 1;
			};

			std::unordered_map<SDL_Keycode, KeyStatus> keys{};
		public:
			bool registerKey(SDL_Keycode key);
			bool unregisterKey(SDL_Keycode key);
			void update();
			void processEvent(const SDL_Event& e);

			inline const KeyStatus& getKeyStatus(SDL_Keycode key) const;
		};

		// checks
		bool isPoked(const Keyboard& keyboard, SDL_KeyCode key);
		bool isReleased(const Keyboard& keyboard, SDL_KeyCode key);
		bool isDown(const Keyboard& keyboard, SDL_KeyCode key);
		bool isHeld(const Keyboard& keyboard, SDL_KeyCode key);
	
	}

	using keyboard::Keyboard;



}



#endif