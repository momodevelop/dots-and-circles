#ifndef __STATE_KEYBOARD_H__
#define __STATE_KEYBOARD_H__

#include "yuu/inputs/keyboard.h"


namespace app {

	struct App;
	enum struct State : uint8_t;
	namespace states::input {
		struct InputState {
			yuu::input::Keyboard keyboard;
		};
		void init(InputState& state);
		State run(InputState& state);
		void free(InputState& state);
	}

}

using app::App;





#endif