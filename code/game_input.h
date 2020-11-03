#ifndef __GAME_INPUT__
#define __GAME_INPUT__


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

    GameDebugKey_Space,
    GameDebugKey_Exclaimation,
    GameDebugKey_DoubleQuotes,
    GameDebugKey_Hex,
    GameDebugKey_Dollar,
    GameDebugKey_Percent,
    GameDebugKey_Ampersand,
    GameDebugKey_Quote,
    GameDebugKey_OpenBrace,
    GameDebugKey_CloseBrace,
    GameDebugKey_Asterisk,
    GameDebugKey_Plus,
    GameDebugKey_Comma,
    GameDebugKey_Hyphen,
    GameDebugKey_Period,
    GameDebugKey_Slash,
    GameDebugKey_0,
    GameDebugKey_1,
    GameDebugKey_2,
    GameDebugKey_3,
    GameDebugKey_4,
    GameDebugKey_5,
    GameDebugKey_6,
    GameDebugKey_7,
    GameDebugKey_8,
    GameDebugKey_9,
    GameDebugKey_Colon,
    GameDebugKey_Semicolon,
    GameDebugKey_OpenTriBrace,
    GameDebugKey_CloseTriBrace,
    GameDebugKey_QuestionMark,
    GameDebugKey_At,
    GameDebugKey_A,
    GameDebugKey_B,
    GameDebugKey_C,
    GameDebugKey_D,
    GameDebugKey_E,
    GameDebugKey_F,
    GameDebugKey_G,
    GameDebugKey_H,
    GameDebugKey_I,
    GameDebugKey_J,
    GameDebugKey_K,
    GameDebugKey_L,
    GameDebugKey_M,
    GameDebugKey_N,
    GameDebugKey_O,
    GameDebugKey_P,
    GameDebugKey_Q,
    GameDebugKey_R,
    GameDebugKey_S,
    GameDebugKey_T,
    GameDebugKey_U,
    GameDebugKey_V,
    GameDebugKey_W,
    GameDebugKey_X,
    GameDebugKey_Y,
    GameDebugKey_Z,
    GameDebugKey_OpenSqBraces,
    GameDebugKey_BackSlash,
    GameDebugKey_CloseSqBraces,
    GameDebugKey_Caret,
    GameDebugKey_Underscore,
    GameDebugKey_Backtick,
    GameDebugKey_a,
    GameDebugKey_b,
    GameDebugKey_c,
    GameDebugKey_d,
    GameDebugKey_e,
    GameDebugKey_f,
    GameDebugKey_g,
    GameDebugKey_h,
    GameDebugKey_i,
    GameDebugKey_j,
    GameDebugKey_k,
    GameDebugKey_l,
    GameDebugKey_m,
    GameDebugKey_n,
    GameDebugKey_o,
    GameDebugKey_p,
    GameDebugKey_q,
    GameDebugKey_r,
    GameDebugKey_s,
    GameDebugKey_t,
    GameDebugKey_u,
    GameDebugKey_v,
    GameDebugKey_w,
    GameDebugKey_x,
    GameDebugKey_y,
    GameDebugKey_z,
    GameDebugKey_OpenCurlyBraces,
    GameDebugKey_VerticleBar,
    GameDebugKey_CloseCurlyBraces,
    GameDebugKey_Tilde,

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

    game_input_button DebugKeys[GameDebugKey_Count];
};

static inline void
Update(game_input* Input) {
    for (auto&& itr : Input->Buttons) {
        itr.Before = itr.Now;
    }
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
