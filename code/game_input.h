#ifndef __GAME_INPUT__
#define __GAME_INPUT__

#include "mm_core.h"

enum game_debug_key {
    GameDebugKey_F1,
    GameDebugKey_F2,
    GameDebugKey_F3,
    GameDebugKey_F4,
    GameDebugKey_F5,
    GameDebugKey_F6,
    GameDebugKey_F7,
    GameDebugKey_F8,
    GameDebugKey_F9,
    GameDebugKey_F10,
    GameDebugKey_F11,
    GameDebugKey_F12,
    GameDebugKey_Return,
    GameDebugKey_Backspace,

    GameDebugKey_Count
};

// TODO(Momo): Needs to be a bit more generic to be engine code...
struct game_input_button {
    bool Before : 1;
    bool Now : 1;
};

struct game_input {
    union {
        game_input_button Buttons[6];
        struct {
            game_input_button ButtonUp;
            game_input_button ButtonDown;
            game_input_button ButtonRight;
            game_input_button ButtonLeft;
            game_input_button ButtonConfirm;
            game_input_button ButtonSwitch;
        };
    };
#if INTERNAL
    char DebugTextInputBuffer[11]; // How many fingers can a person have??
    usize DebugTextInputBufferLength;
    game_input_button DebugKeys[GameDebugKey_Count];
#endif
};

#if INTERNAL
static inline void
PushDebugTextInputBuffer(game_input* Input, const char* Str) {
    
    for(; *Str != 0 && Input->DebugTextInputBufferLength < ArrayCount(Input->DebugTextInputBuffer) - 1; 
            ++Str, ++Input->DebugTextInputBufferLength) 
    {
        Input->DebugTextInputBuffer[Input->DebugTextInputBufferLength] = (*Str); 
    }
    Input->DebugTextInputBuffer[Input->DebugTextInputBufferLength] = 0;
}
#endif

static inline void
Update(game_input* Input) {
    for (auto&& itr : Input->Buttons) {
        itr.Before = itr.Now;
    }

#if INTERNAL
    Input->DebugTextInputBuffer[0] = 0;
    Input->DebugTextInputBufferLength = 0;
    for (auto&& itr : Input->DebugKeys) {
        itr.Before = itr.Now;
    }
#endif
}

// before: 0, now: 1
bool IsPoked(game_input_button Button) {
    return !Button.Before && Button.Now;
}

// before: 1, now: 0
bool IsReleased(game_input_button Button) {
    return Button.Before && !Button.Now;
}


// before: X, now: 1
bool IsDown(game_input_button Button) {
    return Button.Now;
}

// before: 1, now: 1
bool IsHeld(game_input_button Button) {
    return Button.Before && Button.Now;
}


#endif //game_input_H
