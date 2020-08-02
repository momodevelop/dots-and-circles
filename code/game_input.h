#ifndef __YUU_INPUT__
#define __YUU_INPUT__

// TODO(Momo): Needs to be a bit more generic to be engine code...
struct game_button {
    bool Before : 1;
    bool Now : 1;
};

union game_input {
    game_button Buttons[5];
    struct {
        game_button ButtonUp;
        game_button ButtonDown;
        game_button ButtonRight;
        game_button ButtonLeft;
        game_button ButtonConfirm;
        
#if INTERNAL
        game_button ButtonDebug[10];
#endif
    };
};

static inline void
Update(game_input* Input) {
    for (auto&& itr : Input->Buttons) {
        itr.Before = itr.Now;
    }
}

// before: 0, now: 1
bool IsPoked(game_button Button) {
    return !Button.Before && Button.Now;
}

// before: 1, now: 0
bool IsReleased(game_button Button) {
    return Button.Before && !Button.Now;
}


// before: X, now: 1
bool IsDown(game_button Button) {
    return Button.Now;
}

// before: 1, now: 1
bool IsHeld(game_button Button) {
    return Button.Before && Button.Now;
}


#endif //PLATFORM_INPUT_H
