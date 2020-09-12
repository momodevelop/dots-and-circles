#ifndef __YUU_INPUT__
#define __YUU_INPUT__

// TODO(Momo): Needs to be a bit more generic to be engine code...
struct input_button {
    bool Before : 1;
    bool Now : 1;
};

struct game_input {
    union {
        input_button Buttons[15];
        struct {
            input_button ButtonUp;
            input_button ButtonDown;
            input_button ButtonRight;
            input_button ButtonLeft;
            input_button ButtonConfirm;
            
#if INTERNAL
            input_button ButtonDebug[10];
#endif
        };
    };
};

static inline void
Update(game_input* Input) {
    for (auto&& itr : Input->Buttons) {
        itr.Before = itr.Now;
    }
}

// before: 0, now: 1
bool IsPoked(input_button Button) {
    return !Button.Before && Button.Now;
}

// before: 1, now: 0
bool IsReleased(input_button Button) {
    return Button.Before && !Button.Now;
}


// before: X, now: 1
bool IsDown(input_button Button) {
    return Button.Now;
}

// before: 1, now: 1
bool IsHeld(input_button Button) {
    return Button.Before && Button.Now;
}


#endif //PLATFORM_INPUT_H
